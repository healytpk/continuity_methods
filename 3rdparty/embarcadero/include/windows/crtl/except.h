/*  except.h

    Definitions for exception handling

*/

/*
 *      C/C++ Run Time Library - Version 29.0
 *
 *      Copyright (c) 1992, 2021 by Embarcadero Technologies, Inc.
 *      All Rights Reserved.
 *
 */

/* $Revision: 37674 $ */

#ifndef __EXCEPT_H
#define __EXCEPT_H
#pragma option push -b -a8 -pc -Vx- -Ve- -w-inl -w-aus -w-sig

#ifndef __cplusplus
#  error Must use C++ for except.h
#endif

#if !defined(__STD_EXCEPTION)
#  include <exception>
#endif

//hack!!!
#include <string>

#if !defined(RC_INVOKED)

#pragma pack(push, 1)
#pragma option push -Vo-     // set standard C++ options

#if defined(__STDC__)
#pragma warn -nak
#endif

#endif  /* !RC_INVOKED */


namespace std {
#if !defined(__clang__) // in exception
typedef void (_RTLENTRY *terminate_handler)();
typedef void (_RTLENTRY *unexpected_handler)();

void _RTLENTRY _NO_RETURN_DECL terminate();
void _RTLENTRY unexpected();
bool  _RTLENTRY uncaught_exception() throw();
#endif
} // std

#ifndef __STDC__
// For backwards compatibility ...
typedef std::unexpected_handler unexpected_function;
typedef std::terminate_handler terminate_function;
#pragma obsolete terminate_function
#pragma obsolete unexpected_function
#endif // !__STDC__

#if !defined(_WIN64)
extern  char *      _RTLENTRY __ThrowFileName();
extern  unsigned    _RTLENTRY __ThrowLineNumber();
extern  char *      _RTLENTRY __ThrowExceptionName();

#define  __throwFileName      __ThrowFileName()
#define  __throwLineNumber    __ThrowLineNumber()
#define  __throwExceptionName __ThrowExceptionName()

#if __cplusplus < 201703L
class _EXPCLASS xmsg : public std::exception
{
public:
    xmsg(const std::string &msg);
    xmsg(const xmsg &);
    virtual ~xmsg() throw();
    xmsg & operator=(const xmsg &);

    virtual const char * what() const throw();
    const std::string & why() const;
    void                raise() throw(xmsg);

private:
    std::string *str;
};

inline const std::string & xmsg::why() const
{
    return *str;
};

/* The xalloc class is here for backwards compatibility ONLY!  Operator new
   will not throw one of these anymore.  Operator new now throws a bad_alloc
   instead.
*/

class _EXPCLASS xalloc : public xmsg
{
public:
    xalloc(const std::string &msg, _SIZE_T size);

    _SIZE_T requested() const;
    void    raise() throw(xalloc);

private:
    _SIZE_T siz;
};

inline xalloc::xalloc(const std::string &msg, _SIZE_T size)
	: xmsg(msg), siz(size)
{
}

inline _SIZE_T xalloc::requested() const
{
    return siz;
}

#pragma obsolete xalloc
#pragma obsolete xmsg
#endif /* __cplusplus < 201703L */
#endif /* _WIN64 */

#if !defined(RC_INVOKED)

#if defined(__STDC__)
#pragma warn .nak
#endif

#pragma option pop      // restore user C++ options
/* restore default packing */
#pragma pack(pop)

#endif  /* !RC_INVOKED */

#pragma option pop
#endif  // __EXCEPT_H

