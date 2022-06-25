/*  direct.h


    Defines structures, macros, and functions for dealing with
    directories and pathnames.

*/

/*
 *      C/C++ Run Time Library - Version 29.0
 *
 *      Copyright (c) 1987, 2021 by Embarcadero Technologies, Inc.
 *      All Rights Reserved.
 *
 */

/* $Revision: 37354 $ */

#ifndef __DIRECT_H
#define __DIRECT_H


#if !defined(__DIR_H)
#include <dir.h>
#endif


#if !defined(RC_INVOKED)

#if defined(__STDC__)
#pragma warn -nak
#endif

#endif  /* !RC_INVOKED */


#ifdef __cplusplus
extern "C" {
#endif

struct _diskfree_t
{
    unsigned total_clusters;
    unsigned avail_clusters;
    unsigned sectors_per_cluster;
    unsigned bytes_per_sector;
};

/* Prototypes for MSVC and renamed POSIX functions */

int    _RTLENTRY _EXPFUNC _chdir(const char _FAR *__path);
int    _RTLENTRY _EXPFUNC _chdrive(int __drive);
char * _RTLENTRY _EXPFUNC _getdcwd(int __drive, char * __buf, int __len);

wchar_t * _RTLENTRY _EXPFUNC _wgetdcwd(int __drive, wchar_t * __buf, int __len);

unsigned long _RTLENTRY _EXPFUNC _getdrives(void);
unsigned      _RTLENTRY _EXPFUNC _getdiskfree(unsigned __drive, struct _diskfree_t * __driveinfo);

#ifdef __cplusplus
}
#endif


#if !defined(RC_INVOKED)

#if defined(__STDC__)
#pragma warn .nak
#endif

#endif  /* !RC_INVOKED */


#endif /* __DIRECT_H */
