/*  malloc.h

    memory management functions and variables.

*/

/*
 *      C/C++ Run Time Library - Version 29.0
 *
 *      Copyright (c) 1991, 2021 by Embarcadero Technologies, Inc.
 *      All Rights Reserved.
 *
 */


/* $Revision: 35623 $ */

#ifndef __MALLOC_H
#define __MALLOC_H
#define _INC_MALLOC  /* MSC Guard name */

#include <alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(RC_INVOKED)

#if defined(__STDC__)
#pragma warn -nak
#endif

#endif  /* !RC_INVOKED */

#if !defined(RC_INVOKED)
#pragma pack(push, 1)
#endif

/* _HEAPINFO structure returned by heapwalk */

typedef struct _heapinfo
{
    int     *_pentry;
    int     *__pentry;
    _SIZE_T  _size;
    int     _useflag;
} _HEAPINFO;


#if !defined(RC_INVOKED)
/* restore default packing */
#pragma pack(pop)
#endif

/* Prototypes */

#if !defined(__clang__)
void *      __cdecl   _EXPFUNC alloca( _SIZE_T __size );
void *      __cdecl            __alloca__ (_SIZE_T __size);

#if (__CGVER__ >= 0x200)
#  define alloca(__size)       __alloca__(__size)
#  define _alloca(__size)      __alloca__(__size)  /* MSC compatible version */
#else
#  define _alloca alloca                           /* MSC compatible version */
#endif

#else

#define _alloca(__size) __builtin_alloca(__size)
#define  alloca(__size) __builtin_alloca(__size)

#endif /* __clang__ */

#if !defined(_WIN64)    
_SIZE_T     _RTLENTRY _EXPFUNC stackavail  (void);
int         _RTLENTRY _EXPFUNC _heapadd    (void * __block, _SIZE_T __size);
int         _RTLENTRY _EXPFUNC _heapchk    (void);
int         _RTLENTRY _EXPFUNC _heapmin    (void);
int         _RTLENTRY _EXPFUNC _heapset    (unsigned int __fill);
int         _RTLENTRY _EXPFUNC _heapwalk   (_HEAPINFO *__entry);
int         _RTLENTRY _EXPFUNC _rtl_heapwalk (_HEAPINFO *__entry);
void *      _RTLENTRY _EXPFUNC _expand     (void * __block, _SIZE_T __size);
_SIZE_T     _RTLENTRY _EXPFUNC _msize      (void * __block );
#endif /* _WIN64 */
    
#ifdef __cplusplus
}
#endif

/* Obsolete functions */
#if !defined(RC_INVOKED) && !defined(_WIN64)
#pragma obsolete _heapwalk
#endif

#if !defined(RC_INVOKED)

#if defined(__STDC__)
#pragma warn .nak
#endif

#endif  /* !RC_INVOKED */


#endif  /* __MALLOC_H */
