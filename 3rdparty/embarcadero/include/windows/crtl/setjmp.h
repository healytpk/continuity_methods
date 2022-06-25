/*  setjmp.h

    Defines typedef and functions for setjmp/longjmp.

*/

/*
 *      C/C++ Run Time Library - Version 29.0
 *
 *      Copyright (c) 1987, 2021 by Embarcadero Technologies, Inc.
 *      All Rights Reserved.
 *
 */

/* $Revision: 37786 $ */

#ifndef __SETJMP_H
#define __SETJMP_H
#define _INC_SETJMP  /* MSC Guard name */

#include <stdint.h>

#if !defined(___DEFS_H)
#include <_defs.h>
#endif

#ifdef __cplusplus
  namespace std {
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(RC_INVOKED)

#if defined(__STDC__)
#pragma warn -nak
#endif

#pragma pack(push, 1)

#endif  /* !RC_INVOKED */

#if !defined(_WIN64)
typedef struct __jmp_buf {
    unsigned    j_ebp;
    unsigned    j_ebx;
    unsigned    j_edi;
    unsigned    j_esi;
    unsigned    j_esp;
    unsigned    j_ret;
    unsigned    j_excep;
    unsigned    j_context;
}   jmp_buf[1];
#else
typedef float __jb128 __attribute__((__vector_size__(16)));

typedef struct __attribute__((aligned(16))) __jmp_buf {
    uint64_t frame;
    uint64_t rbx;
    uint64_t rsp;
    uint64_t rbp;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint32_t mxcsr;
    uint16_t fpcsr;
    uint16_t dummy;
    __jb128 xmm06;
    __jb128 xmm07;
    __jb128 xmm08;
    __jb128 xmm09;
    __jb128 xmm10;
    __jb128 xmm11;
    __jb128 xmm12;
    __jb128 xmm13;
    __jb128 xmm14;
    __jb128 xmm15;
}   jmp_buf[1];
#endif

void    _RTLENTRYF _EXPFUNC _NO_RETURN_DECL longjmp(jmp_buf __jmpb, int __retval);
int     _RTLENTRYF _EXPFUNC setjmp(jmp_buf __jmpb);

#ifndef _SETJMP_DEFINED
#define _SETJMP_DEFINED
#define setjmp(jmp_buf) setjmp(jmp_buf)
#endif

#ifdef __cplusplus
}
#endif

#if !defined(RC_INVOKED)

/* restore default packing */
#pragma pack(pop)

#if defined(__STDC__)
#pragma warn .nak
#endif

#endif  /* !RC_INVOKED */

#ifdef __cplusplus
} // std
#endif /* __cplusplus */

#endif  /* __SETJMP_H */

#if defined(__cplusplus) && !defined(__XUSING_CNAME__) && !defined(__SETJMP_H_USING_LIST)
#define __SETJMP_H_USING_LIST
     using std::jmp_buf;
     using std::longjmp;
     using std::setjmp;
#endif /* __XUSING_CNAME__ */
