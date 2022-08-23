#ifndef _STDARG_H_
#define _STDARG_H_


/* codes from Windows */
typedef char			*va_list;

#define _ADDRESSOF(v)	( &(v) )
#define _INTSIZEOF(n)	( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v)	( ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) )
#define va_arg(ap,t)	( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)		( ap = (va_list)0 )


#endif /* #ifndef _STDARG_H_ */