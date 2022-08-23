#include "chobits.h"
#include "sys_desc.h"

/*
 * DECLARE SYSTEM-SPECIFIC DEFINITIONS
 */
#define DEFAULT_STACK_SIZE			(64*1024) /* 64kbytes */
#define TS_WATCHDOG_CLOCK_POS		(0xb8000+(80-1)*2)

#define PsGetProcessPtr(handle)		((PPROCESS_CONTROL_BLOCK)handle)
#define PsGetThreadPtr(handle)		((PTHREAD_CONTROL_BLOCK)handle)

/*
 * DECLARE STRUCTURES
 */
typedef struct _THREAD_CONTROL_BLOCK {
	HANDLE								parent_process_handle;		/* memory address */

	DWORD								thread_id;					/* thread id */
	HANDLE								thread_handle;				/* memory address */
	THREAD_STATUS						thread_status;				/* thread status */
	BOOL								auto_delete;

	struct _THREAD_CONTROL_BLOCK		*pt_next_thread;			/* next thread point */

	PKSTART_ROUTINE						start_routine;				/* program entry point */
	PVOID								start_context;				/* context to be passed into the entry routine */
	int									*pt_stack_base_address;		/* stack base address */
	DWORD								stack_size;					/* stack size */
	TSS_32								thread_tss32;				/* TSS32 BLOCK */
} THREAD_CONTROL_BLOCK, *PTHREAD_CONTROL_BLOCK;
/* */
typedef struct _PROCESS_CONTROL_BLOCK {
	DWORD								process_id;					/* process id */
	HANDLE								process_handle;				/* memory address */

	struct _PROCESS_CONTROL_BLOCK		*pt_next_process;			/* next process point used by RR-Scheduler */

	DWORD								thread_count;				/* number of threads */
    DWORD								next_thread_id;				/* next thread id used in this process */
	struct _THREAD_CONTROL_BLOCK		*pt_head_thread;			/* first thread point */
} PROCESS_CONTROL_BLOCK, *PPROCESS_CONTROL_BLOCK;
/* */
typedef struct _PROCESS_MANAGER_BLOCK {
	DWORD								process_count;				/* number of processes */
	DWORD								next_process_id;			/* next prodess id */
	struct _THREAD_CONTROL_BLOCK		*pt_current_thread;			/* running thread */
	struct _PROCESS_CONTROL_BLOCK		*pt_head_process;			/* first process point */
} PROCESS_MANAGER_BLOCK, *PPROCESS_MANAGER_BLOCK;

#define MAX_CUTTING_ITEM				30
typedef struct _CUTTING_LIST {
	BYTE								count;
	BYTE								head;
	BYTE								tail;
	HANDLE								handle_list[MAX_CUTTING_ITEM];
} CUTTING_LIST, *PCUTTING_LIST;

/*
 * EXTERNEL FUCTIONS
 */
extern BOOL HalSetupTSS(TSS_32 *pTss32, BOOL IsKernelTSS, int EntryPoint, int *pStackBase, DWORD StackSize);
extern BOOL HalWriteTssIntoGdt(TSS_32 *pTss32, DWORD TssSize, DWORD TssNumber, BOOL SetBusy);
extern VOID HalChangeNtFlag(TSS_32 *pTss32, BOOL SetBit);
extern VOID HalChangeTssBusyBit(WORD TssSeg, BOOL SetBit);
extern BOOL HalSetupTaskLink(TSS_32 *pTss32, WORD TaskLink);

/*
 * GLOBAL FUNCTIONS
 */
BOOL PskInitializeProcessManager(VOID);

/*
 * INTERNEL FUNCTIONS
 */
static DWORD  PspGetNextProcessID(void);
static BOOL   PspAddNewProcess(HANDLE ProcessHandle);
static DWORD  PspGetNextThreadID(HANDLE ProcessHandle);
static BOOL   PspAddNewThread(HANDLE ProcessHandle, HANDLE ThreadHandle);
static HANDLE PspFindNextThreadScheduled(void);

static BOOL PspPopCuttingItem(CUTTING_LIST *pCuttingList, HANDLE *pItem);
static BOOL PspPushCuttingItem(CUTTING_LIST *pCuttingList, HANDLE Item);

static void PspSetupTaskSWEnv(void);

/*
 * INTERNEL PROCESSs
 */
static void PspTaskEntryPoint(void);
static BOOL PspCreateSystemProcess(void); /* threads in system process will not excuted by scheduler */

/*
 * INTERNEL THREADs
 */
static DWORD PspIdleThread(PVOID StartContext);
static DWORD PspProcessCutterThread(PVOID StartContext);
static DWORD PspThreadCutterThread(PVOID StartContext);

/*
 * GLOBAL VARIABLES
 */
static PROCESS_MANAGER_BLOCK m_ProcMgrBlk;
static CUTTING_LIST m_ProcessCuttingList;
static CUTTING_LIST m_ThreadCuttingList;

static BOOL m_bShowTSWatchdogClock;
static DWORD m_TickCount;


/**********************************************************************************************************
 *                                             GLOBAL FUNTIONS                                            *
 **********************************************************************************************************/
BOOL PskInitializeProcessManager(VOID)
{
	/* initialize global variable */
	m_ProcMgrBlk.process_count		= 0;
	m_ProcMgrBlk.next_process_id	= 0;
	m_ProcMgrBlk.pt_current_thread	= 0;
	m_ProcMgrBlk.pt_head_process	= NULL;

	m_ProcessCuttingList.count		= 0;
	m_ProcessCuttingList.head		= 0;
	m_ProcessCuttingList.tail		= 0;

	m_ThreadCuttingList.count		= 0;
	m_ThreadCuttingList.head		= 0;
	m_ThreadCuttingList.tail		= 0;

	m_bShowTSWatchdogClock			= TRUE;
	m_TickCount						= 0;

	if(!PspCreateSystemProcess()) {
		DbgPrint("PspCreateSystemProcess() returned an error.\r\n");
		return FALSE;
	}

	return TRUE;
}


/**********************************************************************************************************
 *                                            TASK ENTRY POINT                                            *
 **********************************************************************************************************/
static void PspTaskEntryPoint(void)
{
	PKSTART_ROUTINE start_routine;
	HANDLE current_thread;
	DWORD ret_value;

	current_thread = PsGetCurrentThread();
	start_routine = PsGetThreadPtr(current_thread)->start_routine;

	/* run */
	ret_value = start_routine(PsGetThreadPtr(current_thread)->start_context);

	/* terminated */
	PsGetThreadPtr(current_thread)->thread_status = THREAD_STATUS_TERMINATED;
	HalTaskSwitch();

	while(1) ;
}


/**********************************************************************************************************
 *                                           EXPORTED FUNTIONS                                            *
 **********************************************************************************************************/
KERNELAPI BOOL PsCreateProcess(OUT PHANDLE ProcessHandle)
{
	PPROCESS_CONTROL_BLOCK pProcess;

	pProcess = MmAllocateNonCachedMemory(sizeof(PROCESS_CONTROL_BLOCK));
	if(pProcess == NULL) return FALSE;

	pProcess->process_id		= PspGetNextProcessID();
	pProcess->process_handle	= (HANDLE)pProcess;
	pProcess->pt_next_process	= NULL;
	pProcess->thread_count		= 0;
	pProcess->next_thread_id	= 0;
	pProcess->pt_head_thread	= NULL;
	if(!PspAddNewProcess((HANDLE)pProcess)) return FALSE;

	*ProcessHandle = pProcess;

	return TRUE;
}

KERNELAPI HANDLE PsGetParentProcess(IN HANDLE ThreadHandle)
{
	return (HANDLE)(PsGetThreadPtr(ThreadHandle)->parent_process_handle);
}

KERNELAPI BOOL PsCreateThread(OUT PHANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PKSTART_ROUTINE StartRoutine, 
					 IN PVOID StartContext, IN DWORD StackSize, IN BOOL AutoDelete)
{
	PTHREAD_CONTROL_BLOCK pThread;
	int *pStack;

	pThread = MmAllocateNonCachedMemory(sizeof(THREAD_CONTROL_BLOCK));
	if(pThread == NULL) return FALSE;
	pStack  = MmAllocateNonCachedMemory(StackSize);
	if(pStack == NULL) return FALSE;

	pThread->parent_process_handle		= ProcessHandle;
	pThread->thread_id					= PspGetNextThreadID(ProcessHandle);
	pThread->thread_handle				= (HANDLE)pThread;
	pThread->thread_status				= THREAD_STATUS_STOP;
	pThread->auto_delete				= AutoDelete;
	pThread->pt_next_thread				= NULL;
	pThread->start_routine				= StartRoutine;
	pThread->start_context				= StartContext;
	pThread->pt_stack_base_address		= pStack;
	pThread->stack_size					= StackSize;
	if(!PspAddNewThread(ProcessHandle, (HANDLE)pThread)) return FALSE;

	HalSetupTSS(&pThread->thread_tss32, TRUE, (int)PspTaskEntryPoint, pStack, StackSize);

	*ThreadHandle = pThread;

	return TRUE;
}

KERNELAPI BOOL PsCreateIntThread(OUT PHANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PKSTART_ROUTINE StartRoutine,
					   IN PVOID StartContext, IN DWORD StackSize)
{
	PTHREAD_CONTROL_BLOCK pThread;
	int *pStack;

	pThread = MmAllocateNonCachedMemory(sizeof(THREAD_CONTROL_BLOCK));
	if(pThread == NULL) return FALSE;
	pStack  = MmAllocateNonCachedMemory(StackSize);
	if(pStack == NULL) return FALSE;

	pThread->parent_process_handle		= ProcessHandle;
	pThread->thread_id					= PspGetNextThreadID(ProcessHandle);
	pThread->thread_handle				= (HANDLE)pThread;
	pThread->thread_status				= THREAD_STATUS_STOP;
	pThread->auto_delete				= FALSE;
	pThread->pt_next_thread				= NULL;
	pThread->start_routine				= StartRoutine;
	pThread->start_context				= StartContext;
	pThread->pt_stack_base_address		= pStack;
	pThread->stack_size					= StackSize;
	if(!PspAddNewThread(ProcessHandle, (HANDLE)pThread)) return FALSE;

	HalSetupTSS(&pThread->thread_tss32, TRUE, (int)StartRoutine, pStack, StackSize);

	*ThreadHandle = pThread;

	return TRUE;
}

KERNELAPI BOOL PsCreateUserThread(OUT PHANDLE ThreadHandle, IN HANDLE ProcessHandle, IN PVOID StartContext)
{
#define USER_APP_ENTRY_POINT			0x00101000
#define USER_APP_STACK_PTR				0x001f0000
#define USER_APP_STACK_SIZE				(1024*64)
	PTHREAD_CONTROL_BLOCK pThread;

	pThread = MmAllocateNonCachedMemory(sizeof(THREAD_CONTROL_BLOCK));
	if(pThread == NULL) return FALSE;

	pThread->parent_process_handle		= ProcessHandle;
	pThread->thread_id					= PspGetNextThreadID(ProcessHandle);
	pThread->thread_handle				= (HANDLE)pThread;
	pThread->thread_status				= THREAD_STATUS_STOP;
	pThread->auto_delete				= FALSE;
	pThread->pt_next_thread				= NULL;
	pThread->start_routine				= (PKSTART_ROUTINE)USER_APP_ENTRY_POINT;
	pThread->start_context				= StartContext;
	pThread->pt_stack_base_address		= (int *)USER_APP_STACK_PTR;
	pThread->stack_size					= USER_APP_STACK_SIZE;
	if(!PspAddNewThread(ProcessHandle, (HANDLE)pThread)) return FALSE;

	HalSetupTSS(&pThread->thread_tss32, TRUE, USER_APP_ENTRY_POINT, (int *)USER_APP_STACK_PTR, USER_APP_STACK_SIZE);

	*ThreadHandle = pThread;

	return TRUE;
}

KERNELAPI THREAD_STATUS PsGetThreadStatus(IN HANDLE ThreadHandle)
{
	return (THREAD_STATUS)(PsGetThreadPtr(ThreadHandle)->thread_status);
}

KERNELAPI BOOL PsSetThreadStatus(HANDLE ThreadHandle, THREAD_STATUS Status)
{
	PsGetThreadPtr(ThreadHandle)->thread_status = Status;

	return TRUE;
}

KERNELAPI BOOL PsDeleteProcess(IN HANDLE ProcessHandle)
{
	return PspPushCuttingItem(&m_ProcessCuttingList, ProcessHandle);
}

KERNELAPI BOOL PsDeleteThread(HANDLE ThreadHandle)
{
	return PspPushCuttingItem(&m_ThreadCuttingList, ThreadHandle);
}

KERNELAPI HANDLE PsGetCurrentThread(VOID)
{
	HANDLE thread;

ENTER_CRITICAL_SECTION();
	thread = (HANDLE)(m_ProcMgrBlk.pt_current_thread);
EXIT_CRITICAL_SECTION();

	return thread;
}

KERNELAPI BOOL PsShowTSWachdogClock(BOOL Show)
{
	m_bShowTSWatchdogClock = Show;
	if(!Show)
		*((BYTE *)TS_WATCHDOG_CLOCK_POS) = ' ';

	return TRUE;
}

KERNELAPI DWORD  PsGetTickCount(VOID)
{
	return m_TickCount;
}

KERNELAPI DWORD  PsGetMilliSec(DWORD TickCount)
{
	/* generally, i tick = 20 milli sec */
	return (DWORD)(TickCount*(1000/TIMEOUT_PER_SECOND));
}


/**********************************************************************************************************
 *                                           INTERNAL FUNTIONS                                            *
 **********************************************************************************************************/
static BOOL PspPopCuttingItem(CUTTING_LIST *pCuttingList, HANDLE *pItem)
{
	BOOL bResult = TRUE;

ENTER_CRITICAL_SECTION();
	{
		/* check up count */
		if(pCuttingList->count == 0) {
			bResult = FALSE;
			goto $exit;
		}

		/* process */
		pCuttingList->count--;
		*pItem = pCuttingList->handle_list[pCuttingList->head++];
		if(pCuttingList->head >= MAX_CUTTING_ITEM)
			pCuttingList->head = 0;
	}
$exit:
EXIT_CRITICAL_SECTION();
	return bResult;
}

static BOOL PspPushCuttingItem(CUTTING_LIST *pCuttingList, HANDLE Item)
{
	BOOL bResult = TRUE;

ENTER_CRITICAL_SECTION();
	{
		/* check up the remain space of Q */
		if(pCuttingList->count >= MAX_CUTTING_ITEM) {
			bResult = FALSE;
			goto $exit;
		}

		/* process */
		pCuttingList->count++;
		pCuttingList->handle_list[pCuttingList->tail++] = Item;
		if(pCuttingList->tail >= MAX_CUTTING_ITEM)
			pCuttingList->tail = 0;
	}
$exit:
EXIT_CRITICAL_SECTION();
	return bResult;
}

static DWORD PspGetNextProcessID(void)
{
	DWORD process_id;

ENTER_CRITICAL_SECTION();
	process_id = m_ProcMgrBlk.next_process_id++;
EXIT_CRITICAL_SECTION();

	return process_id;
}

static DWORD PspGetNextThreadID(HANDLE ProcessHandle)
{
	DWORD thread_id;

ENTER_CRITICAL_SECTION();
	thread_id = PsGetProcessPtr(ProcessHandle)->next_thread_id++;
EXIT_CRITICAL_SECTION();

	return thread_id;
}

static BOOL PspAddNewProcess(HANDLE ProcessHandle)
{
	PPROCESS_CONTROL_BLOCK *pt_next_process;

ENTER_CRITICAL_SECTION();
	pt_next_process = &m_ProcMgrBlk.pt_head_process;
	while(*pt_next_process)
		pt_next_process = &(*pt_next_process)->pt_next_process;
	*pt_next_process = PsGetProcessPtr(ProcessHandle);
	m_ProcMgrBlk.process_count++;
EXIT_CRITICAL_SECTION();

	return TRUE;
}

static BOOL PspAddNewThread(HANDLE ProcessHandle, HANDLE ThreadHandle)
{
	PTHREAD_CONTROL_BLOCK *pt_next_thread;

ENTER_CRITICAL_SECTION();
	pt_next_thread = &PsGetProcessPtr(ProcessHandle)->pt_head_thread;
	while(*pt_next_thread)
		pt_next_thread = &(*pt_next_thread)->pt_next_thread;
	*pt_next_thread = PsGetThreadPtr(ThreadHandle);
	PsGetProcessPtr(ProcessHandle)->thread_count++;
EXIT_CRITICAL_SECTION();

	return TRUE;
}

static HANDLE PspFindNextThreadScheduled(void)
{
	PTHREAD_CONTROL_BLOCK	pt_thread;
	PPROCESS_CONTROL_BLOCK	pt_process;

	if(m_ProcMgrBlk.process_count == 0 || m_ProcMgrBlk.pt_current_thread == NULL || m_ProcMgrBlk.pt_head_process == NULL) {
		return NULL;
	}

	/* if there are no threads to be scheduled, this fuction never returns */
	pt_thread = m_ProcMgrBlk.pt_current_thread;
$find_thread:
	if(pt_thread->pt_next_thread != NULL) {
		pt_thread = pt_thread->pt_next_thread;
	} else {
		while(1) {
			pt_process = PsGetProcessPtr(pt_thread->parent_process_handle)->pt_next_process;
$find_process:
			if(pt_process == NULL)
				pt_process = m_ProcMgrBlk.pt_head_process;
			if(pt_process->pt_head_thread == NULL) {
				pt_process = pt_process->pt_next_process;
				goto $find_process;
			} else {
				pt_thread = pt_process->pt_head_thread;
				break;
			}
		} /* while(1) */
	}
	if(pt_thread->thread_status != THREAD_STATUS_READY && pt_thread->thread_status != THREAD_STATUS_RUNNING)
		goto $find_thread;
	m_ProcMgrBlk.pt_current_thread = pt_thread;		/* replace the current thread handle */

	return (HANDLE)pt_thread;
}

static void PspSetupTaskSWEnv(void)
{
	HANDLE current_thread, next_thread;

	/* get threads */
	current_thread = PsGetCurrentThread();
	next_thread = PspFindNextThreadScheduled();	/* at this time, current thread is changed with new thing */

	if(PsGetThreadPtr(current_thread)->thread_status == THREAD_STATUS_TERMINATED) {
		/* auto delete? */
		if(PsGetThreadPtr(current_thread)->auto_delete) {
			PsDeleteThread(current_thread);
		}
	} else if(PsGetThreadPtr(current_thread)->thread_status == THREAD_STATUS_RUNNING) {
		PsGetThreadPtr(current_thread)->thread_status = THREAD_STATUS_READY;
	}

	/* task switching */
	if(current_thread != next_thread && next_thread != NULL) {
		HalWriteTssIntoGdt(&PsGetThreadPtr(next_thread)->thread_tss32, sizeof(TSS_32), TASK_SW_SEG, TRUE);
		PsGetThreadPtr(next_thread)->thread_status = THREAD_STATUS_RUNNING;
	}
}


/**********************************************************************************************************
 *                                              IDLE PROCESS                                              *
 **********************************************************************************************************/
static DWORD PspIdleThread(PVOID StartContext)
{
	while(1) {
		HalTaskSwitch();
	}

	return 0;
}

static DWORD PspProcessCutterThread(PVOID StartContext)
{
	HANDLE ProcessHandle;
	PPROCESS_CONTROL_BLOCK *pt_prev_process, *pt_cur_process;
	PTHREAD_CONTROL_BLOCK  *pt_cur_thread;

	while(1) {
		/* check process cutting list */
		if(!PspPopCuttingItem(&m_ProcessCuttingList, &ProcessHandle)) {
			HalTaskSwitch();
			continue;
		}

ENTER_CRITICAL_SECTION();
		/* if requsted process handle is same with system process handle, then do nothing!! protect system process */
		if(ProcessHandle == PsGetThreadPtr(PsGetCurrentThread())->parent_process_handle) {
			goto $exit;
		}

		/* find requested process position in the process list */
		pt_prev_process = pt_cur_process = &(m_ProcMgrBlk.pt_head_process);
		while(*pt_cur_process != PsGetProcessPtr(ProcessHandle)) {
			/* there is no requested process in the list */
			if((*pt_cur_process)->pt_next_process == NULL) {
				goto $exit;
			}
			pt_prev_process = pt_cur_process;
			pt_cur_process = &((*pt_cur_process)->pt_next_process);
		}

		/* change next process pointer */
		(*pt_prev_process)->pt_next_process = (*pt_cur_process)->pt_next_process;
		m_ProcMgrBlk.process_count--;

		/* dealloc all threads belonged to the requested process */
		pt_cur_thread = &(PsGetProcessPtr(ProcessHandle)->pt_head_thread);
		while(*pt_cur_thread != NULL) {
			MmFreeNonCachedMemory((PVOID)((*pt_cur_thread)->pt_stack_base_address));
			MmFreeNonCachedMemory((PVOID)(*pt_cur_thread));
			pt_cur_thread = &((*pt_cur_thread)->pt_next_thread);
		}

		/* dealloc the requested process memory */
		MmFreeNonCachedMemory((PVOID)ProcessHandle);

$exit:
EXIT_CRITICAL_SECTION();
	}

	return 0;
}

static DWORD PspThreadCutterThread(PVOID StartContext)
{
	HANDLE ProcessHandle, ThreadHandle;
	PTHREAD_CONTROL_BLOCK *pt_prev_thread, *pt_cur_thread;

	while(1) {
		/* check thread cutting list */
		if(!PspPopCuttingItem(&m_ThreadCuttingList, &ThreadHandle)) {
			HalTaskSwitch();
			continue;
		}

ENTER_CRITICAL_SECTION();
		ProcessHandle = PsGetThreadPtr(ThreadHandle)->parent_process_handle;

		/* if requsted thread's parent process handle is same with system process handle, then do nothing!! protect system thread */
		if(ProcessHandle == PsGetThreadPtr(PsGetCurrentThread())->parent_process_handle) {
			goto $exit;
		}

		/* invalid thread */
		if(PsGetProcessPtr(ProcessHandle)->thread_count == 0) {
			goto $exit;
		} 
		/* check whether there is only one thread exist in the parent process */
		else if(PsGetProcessPtr(ProcessHandle)->thread_count == 1) {
			PsGetProcessPtr(ProcessHandle)->pt_head_thread = NULL;
		}
		/* more than 2 threads exist */
		else {
			pt_prev_thread = pt_cur_thread = &(PsGetProcessPtr(ProcessHandle)->pt_head_thread);
			while(*pt_cur_thread != PsGetThreadPtr(ThreadHandle)) {
				/* there is no requested thread in the parent process */
				if((*pt_cur_thread)->pt_next_thread == NULL) {
					goto $exit;
				}
				pt_prev_thread = pt_cur_thread;
				pt_cur_thread = &((*pt_cur_thread)->pt_next_thread);
			}
			/* change next thread pointer */
			(*pt_prev_thread)->pt_next_thread = (*pt_cur_thread)->pt_next_thread;
		}
		PsGetProcessPtr(ProcessHandle)->thread_count--;

		/* except user mode program's stack */
		if(PsGetThreadPtr(ThreadHandle)->pt_stack_base_address >= (int *)0x00200000)
			MmFreeNonCachedMemory((PVOID)(PsGetThreadPtr(ThreadHandle)->pt_stack_base_address)); /* dealloc stack */
		MmFreeNonCachedMemory((PVOID)(PsGetThreadPtr(ThreadHandle))); /* dealloc thread */

$exit:
EXIT_CRITICAL_SECTION();
	}

	return 0;
}

static DWORD PspSoftTaskSW(PVOID StartContext)
{
	int cnt=0, pos=0;
	char *addr=(char *)TS_WATCHDOG_CLOCK_POS, status[] = {'-', '\\', '|', '/', '-', '\\', '|', '/'};

	while(1) {
		_asm cli

		/* draw alalog watch-dog clock */
		if(cnt++ >= TIMEOUT_PER_SECOND) {
			if(++pos > 7) pos = 0;
			cnt = 0;
			if(m_bShowTSWatchdogClock)
				*addr = status[pos];
		}

		PspSetupTaskSWEnv();

		_asm iretd
	}

	return 0;
}

static DWORD Psp_IRQ_SystemTimer(PVOID StartContext)
{
	while(1) {
		_asm cli

		m_TickCount++; /* increase tick count */
		PspSetupTaskSWEnv(); /* task-switching */
        WRITE_PORT_UCHAR((PUCHAR)0x20, 0x20); /* send EOI */

		_asm iretd
	}

	return 0;
}

static BOOL PspCreateSystemProcess(void)
{
	HANDLE process_handle;
	HANDLE init_thread_handle, idle_thread_handle, process_cutter_handle, thread_cutter_handle;
	HANDLE tmr_thread_handle, sw_task_sw_handle;

	if(!PsCreateProcess(&process_handle)) 
		return FALSE;

	/* create init_thread. */
	if(!PsCreateThread(&init_thread_handle, process_handle, NULL, NULL, DEFAULT_STACK_SIZE, FALSE)) 
		return FALSE;
	HalSetupTaskLink(&PsGetThreadPtr(init_thread_handle)->thread_tss32, TASK_SW_SEG);
	HalWriteTssIntoGdt(&PsGetThreadPtr(init_thread_handle)->thread_tss32, sizeof(TSS_32), INIT_TSS_SEG, FALSE);
	_asm {
		push	ax
		mov		ax, INIT_TSS_SEG
		ltr		ax
		pop		ax
	}

	/* create tmr_int_handler_thread. */
	if(!PsCreateIntThread(&tmr_thread_handle, process_handle, Psp_IRQ_SystemTimer, NULL, DEFAULT_STACK_SIZE)) 
		return FALSE;
	HalWriteTssIntoGdt(&PsGetThreadPtr(tmr_thread_handle)->thread_tss32, sizeof(TSS_32), TMR_TSS_SEG, FALSE);

	/* create soft-task-switching thread */
	if(!PsCreateIntThread(&sw_task_sw_handle, process_handle, PspSoftTaskSW, NULL, DEFAULT_STACK_SIZE)) 
		return FALSE;
	HalWriteTssIntoGdt(&PsGetThreadPtr(sw_task_sw_handle)->thread_tss32, sizeof(TSS_32), SOFT_TS_TSS_SEG, FALSE);

	/* create idle_thread. */
	if(!PsCreateThread(&idle_thread_handle, process_handle, PspIdleThread, NULL, DEFAULT_STACK_SIZE, FALSE)) 
		return FALSE;
	PsSetThreadStatus(idle_thread_handle, THREAD_STATUS_RUNNING);

	HalWriteTssIntoGdt(&PsGetThreadPtr(idle_thread_handle)->thread_tss32, sizeof(TSS_32), TASK_SW_SEG, 
		TRUE); /* LAST PARAMETER SHOULD BE SET WITH 'TRUE'. IMPORTANT!! */
	m_ProcMgrBlk.pt_current_thread = idle_thread_handle; /* IMPORTANT!! */

	/* process cutter & thread cutter */
	if(!PsCreateThread(&process_cutter_handle, process_handle, PspProcessCutterThread, NULL, DEFAULT_STACK_SIZE, FALSE)) 
		return FALSE;
	PsSetThreadStatus(process_cutter_handle, THREAD_STATUS_READY);
	if(!PsCreateThread(&thread_cutter_handle, process_handle, PspThreadCutterThread, NULL, DEFAULT_STACK_SIZE, FALSE)) 
		return FALSE;
	PsSetThreadStatus(thread_cutter_handle, THREAD_STATUS_READY);

	return TRUE;
}