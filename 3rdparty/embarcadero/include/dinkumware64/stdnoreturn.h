/* stdnoreturn.h standard header */
#ifndef _STDNORETURN
#define _STDNORETURN

 #ifndef __cplusplus

  #if 0 < _MSC_VER
  #define noreturn	noreturn

  #else /* 0 < _MSC_VER */
  #define noreturn	_Noreturn
  #endif /* 0 < _MSC_VER */

 #endif /* __cplusplus */
 #endif /* _STDNORETURN */

/*
 * Copyright (c) by P.J. Plauger. All rights reserved.
 * Consult your license regarding permissions and restrictions.
V8.03a/17:1422 */
