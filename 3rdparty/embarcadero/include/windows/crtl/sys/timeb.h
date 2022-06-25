/*  timeb.h

    Struct and function declarations for ftime().

*/

/*
 *      C/C++ Run Time Library - Version 29.0
 *
 *      Copyright (c) 1987, 2021 by Embarcadero Technologies, Inc.
 *      All Rights Reserved.
 *
 */


/* $Revision: 23293 $ */

#if !defined(__TIMEB_H)
#define __TIMEB_H

#if !defined(___DEFS_H)
#include <_defs.h>
#endif


#if !defined(RC_INVOKED)

#if defined(__STDC__)
#pragma warn -nak
#endif

#endif  /* !RC_INVOKED */


struct timeb {
      long  time;
      short millitm;
      short timezone;
      short dstflag;
      };

#ifdef __cplusplus
extern "C" {
#endif
void _RTLENTRY _EXPFUNC ftime(struct timeb _FAR *);
#ifdef __cplusplus
}
#endif


#if !defined(RC_INVOKED)

#if defined(__STDC__)
#pragma warn .nak
#endif


#endif  /* !RC_INVOKED */

#endif  /* __TIMEB_H */
