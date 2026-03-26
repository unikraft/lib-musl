/* SPDX-License-Identifier: BSD-3-Clause */
#ifndef _STDATOMIC_H
#define _STDATOMIC_H

#if !defined(__GNUC__) && !defined(__clang__)
#error "This stdatomic.h implementation requires GCC or Clang"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* Memory order */
typedef enum {
	memory_order_relaxed = __ATOMIC_RELAXED,
	memory_order_consume = __ATOMIC_CONSUME,
	memory_order_acquire = __ATOMIC_ACQUIRE,
	memory_order_release = __ATOMIC_RELEASE,
	memory_order_acq_rel = __ATOMIC_ACQ_REL,
	memory_order_seq_cst = __ATOMIC_SEQ_CST
} memory_order;

/* Atomic type wrapper */
#define __ATOMIC_WRAP(T) \
	struct {         \
		T _value; \
	} __attribute__((aligned(sizeof(T))))

/* Basic atomic types */
typedef __ATOMIC_WRAP(char) atomic_char;
typedef __ATOMIC_WRAP(signed char) atomic_schar;
typedef __ATOMIC_WRAP(unsigned char) atomic_uchar;
typedef __ATOMIC_WRAP(short) atomic_short;
typedef __ATOMIC_WRAP(unsigned short) atomic_ushort;
typedef __ATOMIC_WRAP(int) atomic_int;
typedef __ATOMIC_WRAP(unsigned int) atomic_uint;
typedef __ATOMIC_WRAP(long) atomic_long;
typedef __ATOMIC_WRAP(unsigned long) atomic_ulong;
typedef __ATOMIC_WRAP(long long) atomic_llong;
typedef __ATOMIC_WRAP(unsigned long long) atomic_ullong;

/* Fixed width types */
typedef __ATOMIC_WRAP(int8_t) atomic_int8_t;
typedef __ATOMIC_WRAP(uint8_t) atomic_uint8_t;
typedef __ATOMIC_WRAP(int16_t) atomic_int16_t;
typedef __ATOMIC_WRAP(uint16_t) atomic_uint16_t;
typedef __ATOMIC_WRAP(int32_t) atomic_int32_t;
typedef __ATOMIC_WRAP(uint32_t) atomic_uint32_t;
typedef __ATOMIC_WRAP(int64_t) atomic_int64_t;
typedef __ATOMIC_WRAP(uint64_t) atomic_uint64_t;

/* Least / fast types */
typedef __ATOMIC_WRAP(int_least8_t) atomic_int_least8_t;
typedef __ATOMIC_WRAP(uint_least8_t) atomic_uint_least8_t;
typedef __ATOMIC_WRAP(int_least16_t) atomic_int_least16_t;
typedef __ATOMIC_WRAP(uint_least16_t) atomic_uint_least16_t;
typedef __ATOMIC_WRAP(int_least32_t) atomic_int_least32_t;
typedef __ATOMIC_WRAP(uint_least32_t) atomic_uint_least32_t;
typedef __ATOMIC_WRAP(int_least64_t) atomic_int_least64_t;
typedef __ATOMIC_WRAP(uint_least64_t) atomic_uint_least64_t;

typedef __ATOMIC_WRAP(int_fast8_t) atomic_int_fast8_t;
typedef __ATOMIC_WRAP(uint_fast8_t) atomic_uint_fast8_t;
typedef __ATOMIC_WRAP(int_fast16_t) atomic_int_fast16_t;
typedef __ATOMIC_WRAP(uint_fast16_t) atomic_uint_fast16_t;
typedef __ATOMIC_WRAP(int_fast32_t) atomic_int_fast32_t;
typedef __ATOMIC_WRAP(uint_fast32_t) atomic_uint_fast32_t;
typedef __ATOMIC_WRAP(int_fast64_t) atomic_int_fast64_t;
typedef __ATOMIC_WRAP(uint_fast64_t) atomic_uint_fast64_t;

/* Size and pointer-sized types */
typedef __ATOMIC_WRAP(size_t) atomic_size_t;
typedef __ATOMIC_WRAP(ptrdiff_t) atomic_ptrdiff_t;
typedef __ATOMIC_WRAP(intptr_t) atomic_intptr_t;
typedef __ATOMIC_WRAP(uintptr_t) atomic_uintptr_t;

/* Boolean */
typedef __ATOMIC_WRAP(unsigned char) atomic_bool;

/* atomic_flag */
typedef struct {
	unsigned char _value;
} atomic_flag;

#define ATOMIC_FLAG_INIT { 0 }

/* Lock-free macros */
#define ATOMIC_CHAR_LOCK_FREE 2
#define ATOMIC_SHORT_LOCK_FREE 2
#define ATOMIC_INT_LOCK_FREE 2
#define ATOMIC_LONG_LOCK_FREE 2
#define ATOMIC_LLONG_LOCK_FREE 2
#define ATOMIC_POINTER_LOCK_FREE 2

/* Initialization */
#define ATOMIC_VAR_INIT(val) { ._value = (val) }

static inline void atomic_init(void *obj, int val)
{
	((atomic_int *)obj)->_value = val;
}

/* Load / Store */
#define atomic_load(obj) ({ \
	__typeof__((obj)->_value) __ret; \
	__atomic_load(&(obj)->_value, &__ret, __ATOMIC_SEQ_CST); \
	__ret; \
})

#define atomic_load_explicit(obj, order) ({ \
	__typeof__((obj)->_value) __ret; \
	__atomic_load(&(obj)->_value, &__ret, (order)); \
	__ret; \
})

#define atomic_store(obj, val) do { \
	__typeof__((obj)->_value) __tmp = (val); \
	__atomic_store(&(obj)->_value, &__tmp, __ATOMIC_SEQ_CST); \
} while (0)

#define atomic_store_explicit(obj, val, order) do { \
	__typeof__((obj)->_value) __tmp = (val); \
	__atomic_store(&(obj)->_value, &__tmp, (order)); \
} while (0)

/* Exchange */
#define atomic_exchange(obj, val) ({ \
	__typeof__((obj)->_value) __tmp = (val); \
	__typeof__((obj)->_value) __ret; \
	__atomic_exchange(&(obj)->_value, &__tmp, &__ret, __ATOMIC_SEQ_CST); \
	__ret; \
})

#define atomic_exchange_explicit(obj, val, order) ({ \
	__typeof__((obj)->_value) __tmp = (val); \
	__typeof__((obj)->_value) __ret; \
	__atomic_exchange(&(obj)->_value, &__tmp, &__ret, (order)); \
	__ret; \
})

/* Compare and exchange */
#define atomic_compare_exchange_strong(obj, expected, desired) \
	__atomic_compare_exchange(&(obj)->_value, (expected), \
				  &(desired), 0, \
				  __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

#define atomic_compare_exchange_strong_explicit(obj, expected, desired, \
						success, failure) \
	__atomic_compare_exchange(&(obj)->_value, (expected), \
				  &(desired), 0, \
				  (success), (failure))

#define atomic_compare_exchange_weak(obj, expected, desired) \
	__atomic_compare_exchange(&(obj)->_value, (expected), \
				  &(desired), 1, \
				  __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

#define atomic_compare_exchange_weak_explicit(obj, expected, desired, \
					      success, failure) \
	__atomic_compare_exchange(&(obj)->_value, (expected), \
				  &(desired), 1, \
				  (success), (failure))

/* Fetch operations */
#define atomic_fetch_add(obj, val) \
	__atomic_fetch_add(&(obj)->_value, (val), __ATOMIC_SEQ_CST)
#define atomic_fetch_sub(obj, val) \
	__atomic_fetch_sub(&(obj)->_value, (val), __ATOMIC_SEQ_CST)
#define atomic_fetch_or(obj, val) \
	__atomic_fetch_or(&(obj)->_value, (val), __ATOMIC_SEQ_CST)
#define atomic_fetch_and(obj, val) \
	__atomic_fetch_and(&(obj)->_value, (val), __ATOMIC_SEQ_CST)
#define atomic_fetch_xor(obj, val) \
	__atomic_fetch_xor(&(obj)->_value, (val), __ATOMIC_SEQ_CST)

#define atomic_fetch_add_explicit(obj, val, order) \
	__atomic_fetch_add(&(obj)->_value, (val), (order))
#define atomic_fetch_sub_explicit(obj, val, order) \
	__atomic_fetch_sub(&(obj)->_value, (val), (order))
#define atomic_fetch_or_explicit(obj, val, order) \
	__atomic_fetch_or(&(obj)->_value, (val), (order))
#define atomic_fetch_and_explicit(obj, val, order) \
	__atomic_fetch_and(&(obj)->_value, (val), (order))
#define atomic_fetch_xor_explicit(obj, val, order) \
	__atomic_fetch_xor(&(obj)->_value, (val), (order))

/* atomic_flag */
static inline int atomic_flag_test_and_set(volatile atomic_flag *flag)
{
	return __atomic_test_and_set(&flag->_value, __ATOMIC_SEQ_CST);
}

static inline int atomic_flag_test_and_set_explicit(
	volatile atomic_flag *flag, memory_order order)
{
	return __atomic_test_and_set(&flag->_value, order);
}

static inline void atomic_flag_clear(volatile atomic_flag *flag)
{
	__atomic_clear(&flag->_value, __ATOMIC_SEQ_CST);
}

static inline void atomic_flag_clear_explicit(
	volatile atomic_flag *flag, memory_order order)
{
	__atomic_clear(&flag->_value, order);
}

/* Fences */
static inline void atomic_thread_fence(memory_order order)
{
	__atomic_thread_fence(order);
}

static inline void atomic_signal_fence(memory_order order)
{
	__atomic_signal_fence(order);
}

#ifdef __cplusplus
}
#endif

#endif
