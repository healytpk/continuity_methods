/*  types.h

    Types for dealing with time.

*/

/*
 *      C/C++ Run Time Library - Version 29.0
 *
 *      Copyright (c) 1987, 2021 by Embarcadero Technologies, Inc.
 *      All Rights Reserved.
 *
 */


/* $Revision: 37786 $ */

#if !defined(__TYPES_H)
#define __TYPES_H

#ifndef  __TIME32_T_DEFINED
#define  __TIME32_T_DEFINED
typedef long __time32_t;
#endif

#ifndef  __TIME64_T_DEFINED
#define  __TIME64_T_DEFINED
typedef __int64 __time64_t;
#endif

#ifdef __cplusplus
namespace std {
#endif /* __cplusplus */

#ifndef  _TIME_T
#if defined(_WIN64)
typedef __int64 time_t;
#else
typedef long time_t;
#endif
#ifdef __cplusplus
#  define _TIME_T std::time_t
#else
#  define _TIME_T time_t
#endif /* __cplusplus */
#endif

typedef short dev_t;
typedef short ino_t;
typedef short mode_t;
typedef short nlink_t;
typedef int   uid_t;
typedef int   gid_t;
typedef long  off_t;

#ifdef __cplusplus
} // std
#  ifndef __XUSING_CNAME__
   using std::time_t;
   using std::dev_t;
   using std::ino_t;
   using std::mode_t;
   using std::nlink_t;
   using std::uid_t;
   using std::gid_t;
   using std::off_t;
#endif
#endif /* __cplusplus */


#endif  /* __TYPES_H */

