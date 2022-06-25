/*  sys\utime.h

    Definitions for _utime(), _wutime, _futime functions.

*/

/*
 *      C/C++ Run Time Library - Version 29.0
 *
 *      Copyright (c) 1991, 2021 by Embarcadero Technologies, Inc.
 *      All Rights Reserved.
 *
 */

/* $Revision: 25922 $ */

#ifndef __SYS_UTIME_H
#define __SYS_UTIME_H

#include <_stddef.h>
#include <sys\types.h>

#if !defined(RC_INVOKED)

#if defined(__STDC__)
#pragma warn -nak
#endif

#pragma pack(push, 1)

#endif  /* !RC_INVOKED */

/* Structure passed to utime containing file times
 */
struct _utimbuf
{
        _TIME_T  actime;         /* access time (used on NTFS) */
        _TIME_T  modtime;        /* modification time */
};

#ifdef __cplusplus
extern "C" {
#endif

int    _RTLENTRY _EXPFUNC _utime(const char * __path,
                                 const struct _utimbuf *__times);
int    _RTLENTRY _EXPFUNC _wutime(const wchar_t * __path,
                                  const struct _utimbuf *__times);
int    _RTLENTRY _EXPFUNC _futime(int __fd, const struct _utimbuf *__times);

#ifdef __cplusplus
}
#endif


#if !defined(RC_INVOKED)

#pragma pack(pop)       /* restore default packing */

#if defined(__STDC__)
#pragma warn .nak
#endif

#endif  /* !RC_INVOKED */


#endif  /* __SYS_UTIME_H */
