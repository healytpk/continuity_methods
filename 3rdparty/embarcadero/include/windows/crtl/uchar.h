/*  uchar.h

    Support for C language TR2

*/

/*
 *      C/C++ Run Time Library - Version 29.0
 *
 *      Copyright (c) 2010, 2021 by Embarcadero Technologies, Inc.
 *      All Rights Reserved.
 *
 */

/* $Revision:$ */

#ifndef __UCHAR_H
#define __UCHAR_H

/* Classic compiler directly supports char16_t and char32_t for "C" */

#if !defined(__cplusplus) && defined(__clang__)
#include <stdint.h>

#define __STDC_UTF_16__ 1
#define __STDC_UTF_32__ 1

typedef uint_least16_t char16_t;
typedef uint_least32_t char32_t;
#endif

#endif /* __UCHAR_H */
