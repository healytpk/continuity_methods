#ifndef __WRAP_MM_MALLOC_H
#define __WRAP_MM_MALLOC_H

#if defined(__clang__)
  #if (__clang_major__ == 5 && __clang_minor__ == 0)
    #include <clang5.0\mm_malloc.h>
  #elif (__clang_major__ == 3 && __clang_minor__ == 3)
    #include <clang3.3\mm_malloc.h>
  #else
    #error "Unable to determine correct clang header version"
  #endif
#else
    #error "Only supported for clang compilers"
#endif

#endif /* __WRAP_MM_MALLOC_H */
