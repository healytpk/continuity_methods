/* WCTYPE.H - Most of this stuff is in TCHAR.H, so we include that */

/*
 *      C/C++ Run Time Library - Version 29.0
 *
 *      Copyright (c) 1997, 2021 by Embarcadero Technologies, Inc.
 *      All Rights Reserved.
 *
 */


/* $Revision: 37814 $ */

#ifndef __WCTYPE_H
#define __WCTYPE_H

#if !defined(__TCHAR_H)
#  include <tchar.h>
#endif

#if !defined(__CTYPE_H)
#  include <ctype.h>
#endif

#ifdef __cplusplus
namespace std {
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

wctype_t _RTLENTRY _EXPFUNC wctype(const char *__str);
int _RTLENTRY _EXPFUNC iswctype(wint_t __wc, wctype_t __desc);

#ifdef __cplusplus
}
#endif

#ifndef WEOF
#define WEOF (_WINT_T)(0xFFFF)   /* wide-character end of file indicator */
#endif /* WEOF */

#ifdef __cplusplus
} // std
#endif /* __cplusplus */

#if defined(__cplusplus) && !defined(__XUSING_CNAME__) && !defined(__WCTYPE_H_USING_LIST)
#define __WCTYPE_H_USING_LIST
    using std::iswctype;
    using std::wctype;
#endif

#endif /* __WCTYPE_H */
