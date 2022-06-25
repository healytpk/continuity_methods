#ifndef __WRAP_EMMINTRIN_H
#define __WRAP_EMMINTRIN_H

#if defined(__clang__)
  #if (__clang_major__ == 5 && __clang_minor__ == 0)
    #include <clang5.0\emmintrin.h>
  #elif (__clang_major__ == 3 && __clang_minor__ == 3)
    #include <clang3.3\emmintrin.h>
  #else
    #error "Unable to determine correct clang header version"
  #endif
#else
    #error "Only supported for clang compilers"
#endif

#endif /* __WRAP_EMMINTRIN_H */
