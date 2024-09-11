#ifndef __CVITEK_ATOMIC_H__
#define __CVITEK_ATOMIC_H__

#include <stdint.h>
#include <sys/types.h>
#include <stdatomic.h>

static inline atomic_int_least32_t *to_atomic_int_least32_t(const int32_t *addr)
{
	return (atomic_int_least32_t *)addr;
}

static inline int32_t cvitek_atomic_inc(int32_t *addr)
{
	atomic_int_least32_t *a = to_atomic_int_least32_t(addr);
	/* Int32_t, if it exists, is the same as int_least32_t. */
	return atomic_fetch_add_explicit(a, 1, memory_order_release);
}

static inline int32_t cvitek_atomic_dec(int32_t *addr)
{
	atomic_int_least32_t *a = to_atomic_int_least32_t(addr);

	return atomic_fetch_sub_explicit(a, 1, memory_order_release);
}

static inline int32_t cvitek_atomic_add(int32_t value, int32_t *addr)
{
	atomic_int_least32_t *a = to_atomic_int_least32_t(addr);

	return atomic_fetch_add_explicit(a, value, memory_order_release);
}

static inline int32_t cvitek_atomic_and(int32_t value, int32_t *addr)
{
	atomic_int_least32_t *a = to_atomic_int_least32_t(addr);

	return atomic_fetch_and_explicit(a, value, memory_order_release);
}

static inline int32_t cvitek_atomic_or(int32_t value, int32_t *addr)
{
	atomic_int_least32_t *a = to_atomic_int_least32_t(addr);

	return atomic_fetch_or_explicit(a, value, memory_order_release);
}

static inline int32_t cvitek_atomic_acquire_load(const int32_t *addr)
{
	atomic_int_least32_t *a = to_atomic_int_least32_t(addr);

	return atomic_load_explicit(a, memory_order_acquire);
}

static inline int32_t cvitek_atomic_release_load(const int32_t *addr)
{
	atomic_int_least32_t *a = to_atomic_int_least32_t(addr);

	atomic_thread_fence(memory_order_seq_cst);
	return atomic_load_explicit(a, memory_order_relaxed);
}

static inline void cvitek_atomic_acquire_store(int32_t value, int32_t *addr)
{
	atomic_int_least32_t *a = to_atomic_int_least32_t(addr);

	atomic_store_explicit(a, value, memory_order_relaxed);
	atomic_thread_fence(memory_order_seq_cst);
}

static inline void cvitek_atomic_release_store(int32_t value, int32_t *addr)
{
	atomic_int_least32_t *a = to_atomic_int_least32_t(addr);

	atomic_store_explicit(a, value, memory_order_release);
}

static inline int cvitek_atomic_acquire_cas(int32_t oldvalue, int32_t newvalue,
						   int32_t *addr)
{
	atomic_int_least32_t *a = to_atomic_int_least32_t(addr);

	return !atomic_compare_exchange_strong_explicit(
											a, &oldvalue, newvalue,
											memory_order_acquire,
											memory_order_acquire);
}

static inline int cvitek_atomic_release_cas(int32_t oldvalue, int32_t newvalue,
							   int32_t *addr)
{
	atomic_int_least32_t *a = to_atomic_int_least32_t(addr);

	return !atomic_compare_exchange_strong_explicit(
											a, &oldvalue, newvalue,
											memory_order_release,
											memory_order_relaxed);
}


#endif // __CVITEK_ATOMIC_H__
