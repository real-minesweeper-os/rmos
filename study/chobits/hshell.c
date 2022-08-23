#include "hshell.h"

/*
 * DECLARE DEFINITIONS
 */
#define DEFAULT_STACK_SIZE				(64*1024) /* 64kbytes */
#define HIDEKI_SHELL_VER				"1st released. Sept 6th, 2003."
#define SHELL_PROMPT					"A:\\>"
#define SHELL_PROMPT_LENGTH				strlen(SHELL_PROMPT)

/*
 * GLOBAL FUNCTIONS
 */
BOOL HshInitializeShell(VOID);

/*
 * STRUCTURES
 */
typedef BOOL (*CMD_PROCESS_ROUTINE)(BYTE *pParameters);

typedef struct _INTERNAL_COMMAND {
	BYTE					*pt_cmd;
	CMD_PROCESS_ROUTINE		routine;
} INTERNEL_COMMAND, *PINTERNEL_COMMAND;

/*
 * EXTERNEL FUNCTIONS
 */
extern BOOL FsInitializeModule(VOID);

/*
 * INTERNEL FUNCTIONS
 */
static DWORD HshpMainThread(PVOID StartContext);

static VOID  HshpPrintPrompt(void);

static BOOL  HshpParser(void);
static DWORD Hshp_CMD_excute(BYTE *pCmd, BYTE *pParameters);

static BOOL  Hshp_CMD_help(BYTE *pParameters);
static BOOL  Hshp_CMD_cls(BYTE *pParameters);
static BOOL  Hshp_CMD_ver(BYTE *pParameters);
static BOOL  Hshp_CMD_dir(BYTE *pParameters);
static BOOL  Hshp_CMD_type(BYTE *pParameters);

static BOOL  Hshp_CMD_toggle(BYTE *pParameters);

/*
 * GLOBAL VARIABLES
 */
static HANDLE m_ProcessHandle, m_ThreadHandle;

#define MAX_CMD_LINE_CHAR			128
static BYTE m_CmdLine[MAX_CMD_LINE_CHAR];
static INTERNEL_COMMAND m_InternalCmds[] = {
	"HELP",		Hshp_CMD_help,
	"CLS",		Hshp_CMD_cls,
	"VER",		Hshp_CMD_ver,
	"DIR",		Hshp_CMD_dir,
	"TYPE",		Hshp_CMD_type,

	"TOGGLE",	Hshp_CMD_toggle,

	NULL, NULL,
};


/**********************************************************************************************************
 *                                             GLOBAL FUNTIONS                                            *
 **********************************************************************************************************/
BOOL HshInitializeShell(VOID)
{
	memset(m_CmdLine, NULL, MAX_CMD_LINE_CHAR);

	/* create hideki-shell process */
	if(!PsCreateProcess(&m_ProcessHandle)) 
		return FALSE;

	/* create main thread */
	if(!PsCreateThread(&m_ThreadHandle, m_ProcessHandle, HshpMainThread, NULL, DEFAULT_STACK_SIZE, FALSE)) 
		return FALSE;
	PsSetThreadStatus(m_ThreadHandle, THREAD_STATUS_READY); /* i'm ready! */

	return TRUE;
}


/**********************************************************************************************************
 *                                           INTERNEL FUNTIONS                                            *
 **********************************************************************************************************/
static DWORD HshpMainThread(PVOID StartContext)
{
	KBD_KEY_DATA KeyData;
	BYTE cursor_x, cursor_y;
	static int cmd_next_pos=0;

	/* initialized file system */
	if(!FsInitializeModule()) {
		DbgPrint("FsInitializeModule() returned an error.\r\n");
		return 0;
	}

	HshpPrintPrompt();
	while(1) {
		if(!KbdGetKey(&KeyData)) {
			HalTaskSwitch();
			continue;
		}

		if(KeyData.type != KBD_KTYPE_GENERAL) {
			continue;
		}

		if(KeyData.key == '\b') { /* backspace? */
			CrtGetCursorPos(&cursor_x, &cursor_y);
			if(cursor_x <= SHELL_PROMPT_LENGTH) 
				continue;
			m_CmdLine[--cmd_next_pos] = NULL;
		}

		CrtPrintf("%c", KeyData.key); /* echo */
		/* excute command */
		if(KeyData.key == '\r') {
			m_CmdLine[cmd_next_pos] = NULL;
			HshpParser();
			HshpPrintPrompt();
			cmd_next_pos = 0;
			m_CmdLine[0] = NULL;
		} 
		/* converts tab key to space key */
		else if(KeyData.key == '\t') {
			m_CmdLine[cmd_next_pos++] = ' ';
		}
		/* inserts a key into the internal command line array */
		else if(KeyData.key != '\b') { /* except BACKSPACE */
			m_CmdLine[cmd_next_pos++] = KeyData.key;
		}
	}

	return 0;
}

static VOID  HshpPrintPrompt(void)
{
	CrtPrintText(SHELL_PROMPT);
}

static BOOL  HshpParser(void)
{
	int i;
	CMD_PROCESS_ROUTINE	CmdProcessRoutine;
	BYTE *pt_cmd, *pt_parameters;

	/* no command ? */
	if(m_CmdLine[0] == NULL)
		return TRUE;

	for(i=0; m_CmdLine[i] == ' '; i++) ; /* find first chr */
	pt_cmd = &(m_CmdLine[i]);
	for(++i; m_CmdLine[i] != NULL && m_CmdLine[i] != ' '; i++) ; /* find last pos of command */
	if(m_CmdLine[i] == NULL) {
		pt_parameters = NULL;
		goto $find;
	}
	m_CmdLine[i] = NULL;
	for(++i; m_CmdLine[i] == ' '; i++) ; /* find second chr */
	pt_parameters = &(m_CmdLine[i]);

$find:
	pt_cmd = strupr(pt_cmd);
	for(i=0; m_InternalCmds[i].pt_cmd != NULL; i++) {
		if(strcmp(pt_cmd, m_InternalCmds[i].pt_cmd) == 0) {
			CmdProcessRoutine = m_InternalCmds[i].routine;
			CmdProcessRoutine(pt_parameters);
			return TRUE;
		}
	}

	Hshp_CMD_excute(pt_cmd, pt_parameters);

	return FALSE;
}

static DWORD Hshp_CMD_excute(BYTE *pCmd, BYTE *pParameters)
{
	int length;
	BYTE buffer[8+3+1+1], *pAddr=(BYTE *)0x00100000, *pPos;
	HANDLE hFile, UserThread;

	/* null command ? */
	length = strlen(pCmd);
	if(!length) return 0;

	/* extension hasnt specified ? */
	memset(buffer, 0, 8+3+1+1);
	strcpy(buffer, pCmd);
	strupr(buffer);
	if((pPos = strrchr(buffer, '.')) == NULL) {
		strcat(buffer, ".EXE");
	} else {
		if(strcmp(++pPos, "EXE")) {
			CrtPrintText("ERROR: this is not excutable file. -_-! \r\n");
			return 0;
		}
	}

	/* open file */
	hFile = FsOpenFile(buffer, OF_READ_ONLY);
	if(!hFile) {
		CrtPrintText("ERROR: file open error! \r\n");
		return 0;
	}

	/* load image */
	while(FsReadFile(hFile, pAddr, 256) != 0) {
		pAddr += 256;
	}

	/* run */
	if(!PsCreateUserThread(&UserThread, PsGetParentProcess(PsGetCurrentThread()), NULL)) {
		FsCloseFile(hFile);
		return 0;
	}
	PsSetThreadStatus(UserThread, THREAD_STATUS_READY);
	while(PsGetThreadStatus(UserThread) != THREAD_STATUS_TERMINATED) {
		HalTaskSwitch();
	}
	PsDeleteThread(UserThread);

	/* close file */
	FsCloseFile(hFile);

	return 0;
}


/**********************************************************************************************************
 *                                           OS SPECIFIC COMMANDS                                         *
 **********************************************************************************************************/
static BOOL Hshp_CMD_toggle(BYTE *pParameters)
{
	static BOOL bShow = TRUE;

	bShow = (bShow ? FALSE : TRUE);
	PsShowTSWachdogClock(bShow);

	return TRUE;
}


/**********************************************************************************************************
 *                                        MS-DOS COMPATIBLE COMMANDS                                      *
 **********************************************************************************************************/
static BOOL Hshp_CMD_cls(BYTE *pParameters)
{
	CrtClearScreen();

	return TRUE;
}

static BOOL Hshp_CMD_help(BYTE *pParameters)
{
	CrtPrintText(
		"help    : View available commands and their description. \r\n"
		"ver     : Show the version information of the OS and Hideki shell. \r\n"
		"cls     : Clear screen. \r\n"
		"dir     : Display all files in the current directory. \r\n"
		"type    : Print a text file onto the screen. \r\n"
		"\r\n"
		"toggle  : Show/hide the soft task-switching watchdog clock \r\n"
		"\r\n"
		"Even if you find any bugs or critical errors, DO NOT SEND & TELL ME. \r\n"
		"I have no duties to fix them. :( \r\n"
	);

	return TRUE;
}

static BOOL Hshp_CMD_ver(BYTE *pParameters)
{
	CrtPrintf(
		"OS Version    : Chobits OS   (%s) \r\n"
		"Shell Version : Hideki Shell (%s) \r\n"
		"Developer     : Yeori (Sun Kyung-Ryul) \r\n"
		"e-Mail        : alphamcu@hanmail.net \r\n"
		"Homepage      : www.zap.pe.kr \r\n"
		, CHOBITS_OS_VER, HIDEKI_SHELL_VER
	);

	return TRUE;
}

typedef struct _DIR_CMD_CONTEXT {
	WORD					file_cnt;
	WORD					dir_cnt;
	DWORD					total_file_size;
} DIR_CMD_CONTEXT, *PDIR_CMD_CONTEXT;

static BOOL Hshp_dir_callback(FILE_INFO *pFileInfo, PVOID Context)
{
	WORD year, month, day;
	WORD hour, minute, second;
	BYTE *pAmPm;
	BYTE *pDir;
	PDIR_CMD_CONTEXT pContext = (PDIR_CMD_CONTEXT)Context;

	second = (pFileInfo->time) & 0x001f;
	minute = (pFileInfo->time >> 5) & 0x003f;
	hour = (pFileInfo->time >> 11) & 0x001f;

	if(hour > 12) {
		hour-=12;
		pAmPm = "P.M.";
	} else {
		pAmPm = "A.M.";
	}

	day = (pFileInfo->date) & 0x001f;
	month = (pFileInfo->date >> 5) & 0x000f;
	year = (pFileInfo->date >> 9)+1980;

	if(pFileInfo->attribute & FILE_ATTR_DIRECTORY) {
		pDir = "<DIR>";
		pContext->dir_cnt++;
	} else {
		pDir = "     ";
		pContext->file_cnt++;
	}

	pContext->total_file_size += pFileInfo->filesize;

	CrtPrintf("%04d-%02d-%02d \t %02d:%02d %s  %7d bytes  %s %s \r\n",
		year, month, day, hour, minute, pAmPm, pFileInfo->filesize, pDir, pFileInfo->filename);

	return TRUE;
}

static BOOL  Hshp_CMD_dir(BYTE *pParameters)
{
	DIR_CMD_CONTEXT dir_cmd_context;

	dir_cmd_context.dir_cnt = 0;
	dir_cmd_context.file_cnt = 0;
	dir_cmd_context.total_file_size = 0;

	FsGetFileList(Hshp_dir_callback, &dir_cmd_context);
	CrtPrintf("\t\t\tTotal File Size: %d bytes \t (%d Files) \r\n", 
		dir_cmd_context.total_file_size, dir_cmd_context.file_cnt);
	CrtPrintf("\t\t\tTotal Directories: %d \r\n", dir_cmd_context.dir_cnt);

	return TRUE;
}

static BOOL Hshp_CMD_type(BYTE *pParameters)
{
#define TYPE_BUFFER_SIZE		256
	HANDLE hFile;
	BYTE buffer[TYPE_BUFFER_SIZE+1];
	buffer[TYPE_BUFFER_SIZE]='\0';

	if(pParameters == NULL || strlen(pParameters) == 0) {
		CrtPrintText("ERROR: No selected files. \r\n");
		return FALSE;
	}

	hFile=FsOpenFile(pParameters, OF_READ_ONLY);
	if(!hFile) {
		CrtPrintf("ERROR: '%s' is not exist! \r\n", pParameters);
		return FALSE;
	}

	memset(buffer, 0, TYPE_BUFFER_SIZE);
	while(FsReadFile(hFile, buffer, TYPE_BUFFER_SIZE) != 0) {
		CrtPrintf("%s", buffer);
		memset(buffer, 0, TYPE_BUFFER_SIZE);
	}
	CrtPrintf("\r\n");

	FsCloseFile(hFile);

	return TRUE;
}