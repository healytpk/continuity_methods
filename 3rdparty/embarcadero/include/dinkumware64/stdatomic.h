/* stdatomic.h standard header */
#ifndef _STDATOMIC
#define _STDATOMIC
#include <stdbool.h>

 #ifdef __STDC_NO_ATOMICS__
  #error <stdatomic.h> not supported
 #endif /* __STDC_NO_ATOMICS__ */

#include <stdint.h>
#include <uchar.h>
#include <xatomic.h>

/* defined in xatomic.h:
	_ATOMIC_BOOL_LOCK_FREE
	_ATOMIC_CHAR_LOCK_FREE
	_ATOMIC_CHAR16_LOCK_FREE
	_ATOMIC_CHAR32_LOCK_FREE
	_ATOMIC_WCHAR_LOCK_FREE
	_ATOMIC_SHORT_LOCK_FREE
	_ATOMIC_INT_LOCK_FREE
	_ATOMIC_LONG_LOCK_FREE
	_ATOMIC_LLONG_LOCK_FREE
	_ATOMIC_POINTER_LOCK_FREE

	ATOMIC_FLAG_TEST_ANS_SET
	ATOMIC_FLAG_CLEAR

	_ATOMIC_SIGNAL_FENCE
	_ATOMIC_THREAD_FENCE

	_Atomic_* functions
 */

/* defined in xatomic0.h:
	typedef enum memory_order {
		memory_order_relaxed,
		memory_order_consume,
		memory_order_acquire,
		memory_order_release,
		memory_order_acq_rel,
		memory_order_seq_cst
		} memory_order;
	_Atomic_flag_t
 */

		// LOCK-FREE PROPERTY
#define ATOMIC_CHAR_LOCK_FREE		_ATOMIC_CHAR_LOCK_FREE
#define ATOMIC_CHAR16_T_LOCK_FREE	_ATOMIC_CHAR16_T_LOCK_FREE
#define ATOMIC_CHAR32_T_LOCK_FREE	_ATOMIC_CHAR32_T_LOCK_FREE
#define ATOMIC_WCHAR_T_LOCK_FREE	_ATOMIC_WCHAR_T_LOCK_FREE
#define ATOMIC_SHORT_LOCK_FREE		_ATOMIC_SHORT_LOCK_FREE
#define ATOMIC_INT_LOCK_FREE		_ATOMIC_INT_LOCK_FREE
#define ATOMIC_LONG_LOCK_FREE		_ATOMIC_LONG_LOCK_FREE
#define ATOMIC_LLONG_LOCK_FREE		_ATOMIC_LLONG_LOCK_FREE
#define ATOMIC_FLAG_INIT	{0}
#define ATOMIC_VAR_INIT(val)	{ val }
#define kill_dependency(val)	(val)

typedef _Atomic_flag_t atomic_flag;

		// TYPEDEFS
 #define _ATOMIC_DEF(NAME, TYPE) \
	typedef struct {TYPE _Atom; } atomic_##NAME;
_ATOMIC_DEF(bool, _Bool)
_ATOMIC_DEF(char, char)
_ATOMIC_DEF(schar, signed char)
_ATOMIC_DEF(uchar, unsigned char)
_ATOMIC_DEF(short, short)
_ATOMIC_DEF(ushort, unsigned short)
_ATOMIC_DEF(int, int)
_ATOMIC_DEF(uint, unsigned int)
_ATOMIC_DEF(long, long)
_ATOMIC_DEF(ulong, unsigned long)
_ATOMIC_DEF(llong, long long)
_ATOMIC_DEF(ullong, unsigned long long)
_ATOMIC_DEF(char16_t, char16_t)
_ATOMIC_DEF(char32_t, char32_t)
_ATOMIC_DEF(wchar_t, wchar_t)
_ATOMIC_DEF(int_least8_t, int_least8_t)
_ATOMIC_DEF(uint_least8_t, uint_least8_t)
_ATOMIC_DEF(int_least16_t, int_least16_t)
_ATOMIC_DEF(uint_least16_t, uint_least16_t)
_ATOMIC_DEF(int_least32_t, int_least32_t)
_ATOMIC_DEF(uint_least32_t, uint_least32_t)
_ATOMIC_DEF(int_least64_t, int_least64_t)
_ATOMIC_DEF(uint_least64_t, uint_least64_t)
_ATOMIC_DEF(int_fast8_t, int_fast8_t)
_ATOMIC_DEF(uint_fast8_t, uint_fast8_t)
_ATOMIC_DEF(int_fast16_t, int_fast16_t)
_ATOMIC_DEF(uint_fast16_t, uint_fast16_t)
_ATOMIC_DEF(int_fast32_t, int_fast32_t)
_ATOMIC_DEF(uint_fast32_t, uint_fast32_t)
_ATOMIC_DEF(int_fast64_t, int_fast64_t)
_ATOMIC_DEF(uint_fast64_t, uint_fast64_t)
_ATOMIC_DEF(intptr_t, intptr_t)
_ATOMIC_DEF(uintptr_t, uintptr_t)
_ATOMIC_DEF(size_t, size_t)
_ATOMIC_DEF(ptrdiff_t, ptrdiff_t)
_ATOMIC_DEF(intmax_t, intmax_t)
_ATOMIC_DEF(uintmax_t, uintmax_t)

#undef _ATOMIC_DEF

inline void atomic_thread_fence(memory_order _Order) _NOEXCEPT
	{	// force memory visibility and inhibit compiler reordering
	return (_ATOMIC_THREAD_FENCE(_Order));
	}

inline void atomic_signal_fence(memory_order _Order) _NOEXCEPT
	{	// force memory visibility and inhibit compiler reordering
	return (_ATOMIC_SIGNAL_FENCE(_Order));
	}

		// GENERICS
  #define _GEN_SUF(NAME, VAL) \
	_Generic((VAL) + (uint_least8_t)0, \
		default: _GEN_SUF_2(NAME, VAL), \
			uint_least8_t: NAME##_1)
  #define _GEN_SUF_2(NAME, VAL) \
	_Generic((VAL) + (uint_least16_t)0, \
		default: _GEN_SUF_4(NAME, VAL), \
			uint_least16_t: NAME##_2)
  #define _GEN_SUF_4(NAME, VAL) \
	_Generic((VAL) + (uint_least32_t)0, \
		default: _GEN_SUF_8(NAME, VAL), \
			uint_least32_t: NAME##_4)
  #define _GEN_SUF_8(NAME, VAL) \
	_Generic((VAL) + (uint_least64_t)0, \
		default: NAME##_1, \
			long long: NAME##_8, \
			unsigned long long: NAME##_8)

#define _ATOMIC_GENERICS(SUF, ATYPE, CTYPE) \
inline void _Myatomic_init##SUF(void *_Pobj, CTYPE _Val) \
	{	/* initialize an atomic */ \
	*(volatile ATYPE *)_Pobj = _Val; \
	} \
inline void _Myatomic_store_explicit##SUF(void *_Pobj, CTYPE _Val, \
	memory_order _Ord) \
	{	/* store an atomic */ \
	return (_Atomic_store##SUF((volatile ATYPE *)_Pobj, _Val, _Ord)); \
	} \
inline CTYPE _Myatomic_load_explicit##SUF(const void *_Pobj, \
	memory_order _Ord) \
	{	/* load an atomic */ \
	return (_Atomic_load##SUF((volatile ATYPE *)_Pobj, _Ord)); \
	} \
inline CTYPE _Myatomic_exchange_explicit##SUF(void *_Pobj, CTYPE _Val, \
	memory_order _Ord) \
	{	/* exchange an atomic */ \
	return (_Atomic_exchange##SUF((volatile ATYPE *)_Pobj, _Val, _Ord)); \
	} \
inline CTYPE _Myatomic_compare_exchange_strong_explicit##SUF( \
	void *_Pobj, void *_Pexp, CTYPE _Val, \
		memory_order _Ord1, memory_order _Ord2) \
	{	/* compare/exchange strong an atomic */ \
	return (_Atomic_compare_exchange_strong##SUF( \
		(volatile ATYPE *)_Pobj, (ATYPE *)_Pexp, _Val, \
			_Ord1, _Ord2)); \
	} \
inline CTYPE _Myatomic_compare_exchange_weak_explicit##SUF( \
	void *_Pobj, void *_Pexp, CTYPE _Val, \
		memory_order _Ord1, memory_order _Ord2) \
	{	/* compare/exchange weak an atomic */ \
	return (_Atomic_compare_exchange_weak##SUF( \
		(volatile ATYPE *)_Pobj, (ATYPE *)_Pexp, _Val, \
			_Ord1, _Ord2)); \
	} \
inline CTYPE _Myatomic_fetch_add_explicit##SUF( \
	void *_Pobj, CTYPE _Val, memory_order _Ord) \
	{	/* fetch and add an atomic */ \
	return (_Atomic_fetch_add##SUF( \
		(volatile ATYPE *)_Pobj, _Val, _Ord)); \
	} \
inline CTYPE _Myatomic_fetch_sub_explicit##SUF( \
	void *_Pobj, CTYPE _Val, memory_order _Ord) \
	{	/* fetch and subtract an atomic */ \
	return (_Atomic_fetch_sub##SUF( \
		(volatile ATYPE *)_Pobj, _Val, _Ord)); \
	} \
inline CTYPE _Myatomic_fetch_or_explicit##SUF( \
	void *_Pobj, CTYPE _Val, memory_order _Ord) \
	{	/* fetch and OR an atomic */ \
	return (_Atomic_fetch_or##SUF( \
		(volatile ATYPE *)_Pobj, _Val, _Ord)); \
	} \
inline CTYPE _Myatomic_fetch_xor_explicit##SUF( \
	void *_Pobj, CTYPE _Val, memory_order _Ord) \
	{	/* fetch and XOR an atomic */ \
	return (_Atomic_fetch_xor##SUF( \
		(volatile ATYPE *)_Pobj, _Val, _Ord)); \
	} \
inline CTYPE _Myatomic_fetch_and_explicit##SUF( \
	void *_Pobj, CTYPE _Val, memory_order _Ord) \
	{	/* fetch and AND an atomic */ \
	return (_Atomic_fetch_and##SUF( \
		(volatile ATYPE *)_Pobj, _Val, _Ord)); \
	}

_ATOMIC_GENERICS(_1, _Uint1_t, _Uint1_t)
_ATOMIC_GENERICS(_2, _Uint2_t, _Uint2_t)
_ATOMIC_GENERICS(_4, _Uint4_t, _Uint4_t)
_ATOMIC_GENERICS(_8, _Uint8_t, _Uint8_t)

#undef _ATOMIC_GENERICS

#define atomic_is_lock_free(pobj) \
	_GEN_SUF(_Atomic_is_lock_free, (pobj)->_Atom)()
#define atomic_init(pobj, val) \
	_GEN_SUF(_Myatomic_init, (pobj)->_Atom)(pobj, val)
#define atomic_store(pobj, val) \
	_GEN_SUF(_Myatomic_store_explicit, (pobj)->_Atom)(pobj, val, \
		 memory_order_seq_cst)
#define atomic_store_explicit(pobj, val, ord) \
	_GEN_SUF(_Myatomic_store_explicit, (pobj)->_Atom)(pobj, val, ord)
#define atomic_load(pobj) \
	_GEN_SUF(_Myatomic_load_explicit, (pobj)->_Atom)(pobj,\
		 memory_order_seq_cst)
#define atomic_load_explicit(pobj, ord) \
	_GEN_SUF(_Myatomic_load_explicit, (pobj)->_Atom)(pobj, ord)
#define atomic_exchange(pobj, val) \
	_GEN_SUF(_Myatomic_exchange_explicit, (pobj)->_Atom)(pobj, val, \
		memory_order_seq_cst)
#define atomic_exchange_explicit(pobj, val, ord) \
	_GEN_SUF(_Myatomic_exchange_explicit, (pobj)->_Atom)(pobj, val, ord)
#define atomic_compare_exchange_strong(pobj, pexp, val) \
	_GEN_SUF(_Myatomic_compare_exchange_strong_explicit, (pobj)->_Atom) \
		(pobj, pexp, val, memory_order_seq_cst, memory_order_seq_cst)
#define atomic_compare_exchange_strong_explicit(pobj, pexp, val, ord1, ord2) \
	_GEN_SUF(_Myatomic_compare_exchange_strong_explicit, (pobj)->_Atom) \
		(pobj, pexp, val, ord1, ord2)
#define atomic_compare_exchange_weak(pobj, pexp, val) \
	_GEN_SUF(_Myatomic_compare_exchange_weak_explicit, \
		(pobj)->_Atom)(pobj, pexp, val, \
		memory_order_seq_cst, memory_order_seq_cst)
#define atomic_compare_exchange_weak_explicit(pobj, pexp, val, ord1, ord2) \
	_GEN_SUF(_Myatomic_compare_exchange_weak_explicit, (pobj)->_Atom) \
		(pobj, pexp, val, ord1, ord2)
#define atomic_fetch_add(pobj, val) \
	_GEN_SUF(_Myatomic_fetch_add_explicit, (pobj)->_Atom)(pobj, val, \
		memory_order_seq_cst)
#define atomic_fetch_add_explicit(pobj, val, ord) \
	_GEN_SUF(_Myatomic_fetch_add_explicit, (pobj)->_Atom)(pobj, val, ord)
#define atomic_fetch_sub(pobj, val) \
	_GEN_SUF(_Myatomic_fetch_sub_explicit, (pobj)->_Atom)(pobj, val, \
		memory_order_seq_cst)
#define atomic_fetch_sub_explicit(pobj, val, ord) \
	_GEN_SUF(_Myatomic_fetch_sub_explicit, (pobj)->_Atom)(pobj, val, ord)
#define atomic_fetch_or(pobj, val) \
	_GEN_SUF(_Myatomic_fetch_or_explicit, (pobj)->_Atom)(pobj, val, \
		memory_order_seq_cst)
#define atomic_fetch_or_explicit(pobj, val, ord) \
	_GEN_SUF(_Myatomic_fetch_or_explicit, (pobj)->_Atom)(pobj, val, ord)
#define atomic_fetch_xor(pobj, val) \
	_GEN_SUF(_Myatomic_fetch_xor_explicit, (pobj)->_Atom)(pobj, val, \
		memory_order_seq_cst)
#define atomic_fetch_xor_explicit(pobj, val, ord) \
	_GEN_SUF(_Myatomic_fetch_xor_explicit, (pobj)->_Atom)(pobj, val, ord)
#define atomic_fetch_and(pobj, val) \
	_GEN_SUF(_Myatomic_fetch_and_explicit, (pobj)->_Atom)(pobj, val, \
		memory_order_seq_cst)
#define atomic_fetch_and_explicit(pobj, val, ord) \
	_GEN_SUF(_Myatomic_fetch_and_explicit, (pobj)->_Atom)(pobj, val, ord)

		// atomic_flag FUNCTIONS
inline bool atomic_flag_test_and_set(volatile atomic_flag *_Pflag) _NOEXCEPT
	{	// atomically set *_Pflag to true and return previous value
	return (_ATOMIC_FLAG_TEST_AND_SET(_Pflag,
		memory_order_seq_cst));
	}

inline bool atomic_flag_test_and_set_explicit(
	volatile atomic_flag *_Pflag, memory_order _Ord) _NOEXCEPT
	{	// atomically set *_Pflag to true and return previous value
	return (_ATOMIC_FLAG_TEST_AND_SET(_Pflag, _Ord));
	}

inline void atomic_flag_clear(volatile atomic_flag *_Pflag) _NOEXCEPT
	{	// atomically clear *_Pflag
	_ATOMIC_FLAG_CLEAR(_Pflag, memory_order_seq_cst);
	}

inline void atomic_flag_clear_explicit(
	volatile atomic_flag *_Pflag, memory_order _Ord) _NOEXCEPT
	{	// atomically clear *_Pflag
	return (_ATOMIC_FLAG_CLEAR(_Pflag, _Ord));
	}
#endif /* _STDATOMIC */

/*
 * Copyright (c) by P.J. Plauger. All rights reserved.
 * Consult your license regarding permissions and restrictions.
V8.03a/17:1422 */
