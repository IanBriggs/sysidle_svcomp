#include <stdlib.h>

//POLL.H ADDED
#include <poll.h>

/* 32-bit build. */

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

#define USHRT_MAX	((u16)(~0U))
#define SHRT_MAX	((s16)(USHRT_MAX>>1))
#define SHRT_MIN	((s16)(-SHRT_MAX - 1))
#define INT_MAX		((int)(~0U>>1))
#define INT_MIN		(-INT_MAX - 1)
#define UINT_MAX	(~0U)
#define LONG_MAX	((long)(~0UL>>1))
#define LONG_MIN	(-LONG_MAX - 1)
#define ULONG_MAX	(~0UL)
#define LLONG_MAX	((long long)(~0ULL>>1))
#define LLONG_MIN	(-LLONG_MAX - 1)
#define ULLONG_MAX	(~0ULL)
#define SIZE_MAX	(~(size_t)0)

#define UINT_CMP_GE(a, b)       (UINT_MAX / 2 >= (a) - (b))
#define UINT_CMP_LT(a, b)       (UINT_MAX / 2 < (a) - (b))
#define ULONG_CMP_GE(a, b)      (ULONG_MAX / 2 >= (a) - (b))
#define ULONG_CMP_LT(a, b)      (ULONG_MAX / 2 < (a) - (b))

typedef _Bool                   bool;
#define false 0
#define true  1

#define LOCK_PREFIX "\n\tlock; "

//MOVED TO END OF FILE SO xadd, cmpxchg CAN BE USED
//#include "atomic.h"

/*
 * Non-existant functions to indicate usage errors at link time
 * (or compile-time if the compiler implements __compiletime_error().
 */
#define __compiletime_error(message) __attribute__((error(message)))
extern void __xchg_wrong_size(void)
	__compiletime_error("Bad argument size for xchg");
extern void __cmpxchg_wrong_size(void)
	__compiletime_error("Bad argument size for cmpxchg");
extern void __xadd_wrong_size(void)
	__compiletime_error("Bad argument size for xadd");
extern void __add_wrong_size(void)
	__compiletime_error("Bad argument size for add");

/*
 * Constants for operation sizes. On 32-bit, the 64-bit size it set to
 * -1 because sizeof will never return -1, thereby making those switch
 * case statements guaranteeed dead code which the compiler will
 * eliminate, and allowing the "missing symbol in the default case" to
 * indicate a usage error.
 */
#define __X86_CASE_B	1
#define __X86_CASE_W	2
#define __X86_CASE_L	4
#ifdef CONFIG_64BIT
#define __X86_CASE_Q	8
#else
#define	__X86_CASE_Q	-1		/* sizeof will never return -1 */
#endif

/* 
 * An exchange-type operation, which takes a value and a pointer, and
 * returns a the old value.
 */
#define __xchg_op(ptr, arg, op, lock)					\
  ({ \
  __VERIFIER_atomic_begin(); \
  int temp = (*ptr); \
  (*ptr) = arg; \
  __VERIFIER_atomic_end(); \
  temp;\
}) 

/************************************************************************************/
/* #define __xchg_op(ptr, arg, op, lock)					\   */
/* 	({								\	    */
/* 	        __typeof__ (*(ptr)) __ret = (arg);			\	    */
/* 		switch (sizeof(*(ptr))) {				\	    */
/* 		case __X86_CASE_B:					\	    */
/* 			asm volatile (lock #op "b %b0, %1\n"		\	    */
/* 				      : "+q" (__ret), "+m" (*(ptr))	\	    */
/* 				      : : "memory", "cc");		\	    */
/* 			break;						\	    */
/* 		case __X86_CASE_W:					\	    */
/* 			asm volatile (lock #op "w %w0, %1\n"		\	    */
/* 				      : "+r" (__ret), "+m" (*(ptr))	\	    */
/* 				      : : "memory", "cc");		\	    */
/* 			break;						\	    */
/* 		case __X86_CASE_L:					\	    */
/* 			asm volatile (lock #op "l %0, %1\n"		\	    */
/* 				      : "+r" (__ret), "+m" (*(ptr))	\	    */
/* 				      : : "memory", "cc");		\	    */
/* 			break;						\	    */
/* 		case __X86_CASE_Q:					\	    */
/* 			asm volatile (lock #op "q %q0, %1\n"		\	    */
/* 				      : "+r" (__ret), "+m" (*(ptr))	\	    */
/* 				      : : "memory", "cc");		\	    */
/* 			break;						\	    */
/* 		default:						\	    */
/* 			__ ## op ## _wrong_size();			\	    */
/* 		}							\	    */
/* 		__ret;							\	    */
/* 	})									    */
/************************************************************************************/

/*
 * Note: no "lock" prefix even on SMP: xchg always implies lock anyway.
 * Since this is generally used to protect other memory information, we
 * use "asm volatile" and "memory" clobbers to prevent gcc from moving
 * information around.
 */
#define xchg(ptr, v)	__xchg_op((ptr), (v), xchg, "")

/*
 * Atomic compare and exchange.  Compare OLD with MEM, if identical,
 * store NEW in MEM.  Return the initial value in MEM.  Success is
 * indicated by comparing RETURN with OLD.
 */
#define __raw_cmpxchg(ptr, old, new, size, lock)			\
  ({									\
    if (lock[0] != '\0') {						\
      __VERIFIER_atomic_begin();					\
    }									\
    if ((*ptr) == old) {						\
      (*ptr) = new;							\
    }									\
    if (lock[0] != '\0') {						\
      __VERIFIER_atomic_end();						\
    }									\
    (*ptr);								\
  })

/************************************************************************************/
/* #define __raw_cmpxchg(ptr, old, new, size, lock)			\	    */
/* ({									\	    */
/* 	__typeof__(*(ptr)) __ret;					\	    */
/* 	__typeof__(*(ptr)) __old = (old);				\	    */
/* 	__typeof__(*(ptr)) __new = (new);				\	    */
/* 	switch (size) {							\	    */
/* 	case __X86_CASE_B:						\	    */
/* 	{								\	    */
/* 		volatile u8 *__ptr = (volatile u8 *)(ptr);		\	    */
/* 		asm volatile(lock "cmpxchgb %2,%1"			\	    */
/* 			     : "=a" (__ret), "+m" (*__ptr)		\	    */
/* 			     : "q" (__new), "0" (__old)			\	    */
/* 			     : "memory");				\	    */
/* 		break;							\	    */
/* 	}								\	    */
/* 	case __X86_CASE_W:						\	    */
/* 	{								\	    */
/* 		volatile u16 *__ptr = (volatile u16 *)(ptr);		\	    */
/* 		asm volatile(lock "cmpxchgw %2,%1"			\	    */
/* 			     : "=a" (__ret), "+m" (*__ptr)		\	    */
/* 			     : "r" (__new), "0" (__old)			\	    */
/* 			     : "memory");				\	    */
/* 		break;							\	    */
/* 	}								\	    */
/* 	case __X86_CASE_L:						\	    */
/* 	{								\	    */
/* 		volatile u32 *__ptr = (volatile u32 *)(ptr);		\	    */
/* 		asm volatile(lock "cmpxchgl %2,%1"			\	    */
/* 			     : "=a" (__ret), "+m" (*__ptr)		\	    */
/* 			     : "r" (__new), "0" (__old)			\	    */
/* 			     : "memory");				\	    */
/* 		break;							\	    */
/* 	}								\	    */
/* 	case __X86_CASE_Q:						\	    */
/* 	{								\	    */
/* 		volatile u64 *__ptr = (volatile u64 *)(ptr);		\	    */
/* 		asm volatile(lock "cmpxchgq %2,%1"			\	    */
/* 			     : "=a" (__ret), "+m" (*__ptr)		\	    */
/* 			     : "r" (__new), "0" (__old)			\	    */
/* 			     : "memory");				\	    */
/* 		break;							\	    */
/* 	}								\	    */
/* 	default:							\	    */
/* 		__cmpxchg_wrong_size();					\	    */
/* 	}								\	    */
/* 	__ret;								\	    */
/* })										    */
/************************************************************************************/

#define __cmpxchg(ptr, old, new, size)					\
	__raw_cmpxchg((ptr), (old), (new), (size), LOCK_PREFIX)

#define __sync_cmpxchg(ptr, old, new, size)				\
	__raw_cmpxchg((ptr), (old), (new), (size), "lock; ")

#define __cmpxchg_local(ptr, old, new, size)				\
	__raw_cmpxchg((ptr), (old), (new), (size), "")

#include "cmpxchg_32.h"

#ifdef __HAVE_ARCH_CMPXCHG
#define cmpxchg(ptr, old, new)						\
	__cmpxchg(ptr, old, new, sizeof(*(ptr)))

#define sync_cmpxchg(ptr, old, new)					\
	__sync_cmpxchg(ptr, old, new, sizeof(*(ptr)))

#define cmpxchg_local(ptr, old, new)					\
	__cmpxchg_local(ptr, old, new, sizeof(*(ptr)))
#endif

/*
 * xadd() adds "inc" to "*ptr" and atomically returns the previous
 * value of "*ptr".
 *
 * xadd() is locked when multiple CPUs are online
 * xadd_sync() is always locked
 * xadd_local() is never locked
 */
#define __xadd(ptr, inc, lock)	__xchg_op((ptr), (inc), xadd, lock)
#define xadd(ptr, inc)		__xadd((ptr), (inc), LOCK_PREFIX)
#define xadd_sync(ptr, inc)	__xadd((ptr), (inc), "lock; ")
#define xadd_local(ptr, inc)	__xadd((ptr), (inc), "")

#define __add(ptr, inc, lock)                                           \
  ({									\
    if (lock[0] != '\0') {						\
      __VERIFIER_atomic_begin();					\
    }									\
    (*ptr) += lock;							\
    if (lock[0] != '\0') {						\
      __VERIFIER_atomic_end();						\
    }									\
    (*ptr);								\
  })

  
/************************************************************************************/
/* #define __add(ptr, inc, lock)						\   */
/* 	({								\	    */
/* 	        __typeof__ (*(ptr)) __ret = (inc);			\	    */
/* 		switch (sizeof(*(ptr))) {				\	    */
/* 		case __X86_CASE_B:					\	    */
/* 			asm volatile (lock "addb %b1, %0\n"		\	    */
/* 				      : "+m" (*(ptr)) : "qi" (inc)	\	    */
/* 				      : "memory", "cc");		\	    */
/* 			break;						\	    */
/* 		case __X86_CASE_W:					\	    */
/* 			asm volatile (lock "addw %w1, %0\n"		\	    */
/* 				      : "+m" (*(ptr)) : "ri" (inc)	\	    */
/* 				      : "memory", "cc");		\	    */
/* 			break;						\	    */
/* 		case __X86_CASE_L:					\	    */
/* 			asm volatile (lock "addl %1, %0\n"		\	    */
/* 				      : "+m" (*(ptr)) : "ri" (inc)	\	    */
/* 				      : "memory", "cc");		\	    */
/* 			break;						\	    */
/* 		case __X86_CASE_Q:					\	    */
/* 			asm volatile (lock "addq %1, %0\n"		\	    */
/* 				      : "+m" (*(ptr)) : "ri" (inc)	\	    */
/* 				      : "memory", "cc");		\	    */
/* 			break;						\	    */
/* 		default:						\	    */
/* 			__add_wrong_size();				\	    */
/* 		}							\	    */
/* 		__ret;							\	    */
/* 	})									    */
/************************************************************************************/

/*
 * add_*() adds "inc" to "*ptr"
 *
 * __add() takes a lock prefix
 * add_smp() is locked when multiple CPUs are online
 * add_sync() is always locked
 */
#define add_smp(ptr, inc)	__add((ptr), (inc), LOCK_PREFIX)
#define add_sync(ptr, inc)	__add((ptr), (inc), "lock; ")


//NOT YET IMPLIMENTED
/* #define __cmpxchg_double(pfx, p1, p2, o1, o2, n1, n2)			\ */
/* ({									\ */
/* 	bool __ret;							\ */
/* 	__typeof__(*(p1)) __old1 = (o1), __new1 = (n1);			\ */
/* 	__typeof__(*(p2)) __old2 = (o2), __new2 = (n2);			\ */
/* 	BUILD_BUG_ON(sizeof(*(p1)) != sizeof(long));			\ */
/* 	BUILD_BUG_ON(sizeof(*(p2)) != sizeof(long));			\ */
/* 	VM_BUG_ON((unsigned long)(p1) % (2 * sizeof(long)));		\ */
/* 	VM_BUG_ON((unsigned long)((p1) + 1) != (unsigned long)(p2));	\ */
/* 	asm volatile(pfx "cmpxchg%c4b %2; sete %0"			\ */
/* 		     : "=a" (__ret), "+d" (__old2),			\ */
/* 		       "+m" (*(p1)), "+m" (*(p2))			\ */
/* 		     : "i" (2 * sizeof(long)), "a" (__old1),		\ */
/* 		       "b" (__new1), "c" (__new2));			\ */
/* 	__ret;								\ */
/* }) */

#define cmpxchg_double(p1, p2, o1, o2, n1, n2) \
	__cmpxchg_double(LOCK_PREFIX, p1, p2, o1, o2, n1, n2)

#define cmpxchg_double_local(p1, p2, o1, o2, n1, n2) \
	__cmpxchg_double(, p1, p2, o1, o2, n1, n2)
// WEAK MEMORY MODEL NOT USED IN SVCOMP
#define barrier() //__asm__ __volatile__("": : :"memory")
#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
// WEAK MEMORY MODEL NOT USED IN SVCOMP
#define smp_mb() //asm volatile("mfence":::"memory")

#define likely(x) (x)
#define unlikely(x) (x)

void cpu_relax_poll(void)
{
	poll(NULL, 0, 1);
}

void cpu_relax_poll_random(void)
{
	if ((random() & 0xfff00) == 0)
		poll(NULL, 0, 1);
}

void (*cpu_relax_func)(void) = cpu_relax_poll;

#define cpu_relax() cpu_relax_func()

int __thread my_smp_processor_id;

#define raw_smp_processor_id() my_smp_processor_id

static inline void cpu_init(int cpu)
{
	my_smp_processor_id = cpu;
}

#define WARN_ON(c) \
	do { \
		if (c) \
			abort(); \
	} while (0)
#define BUG_ON(c) \
	do { \
		if (c) \
			abort(); \
	} while (0)

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define DIV_ROUND_UP_ULL(ll,d) \
	({ unsigned long long _tmp = (ll)+(d)-1; do_div(_tmp, d); _tmp; })

#define set_cpus_allowed_ptr(a, b) do { } while (0)

struct rcu_head {
	struct callback_head *next;
	void (*func)(struct callback_head *head);
};

static inline void call_rcu(struct rcu_head *head, void (*func)(struct rcu_head *rcu))
{
	func(head);  /* Don't try this at home!!!  It will normally break. */
}

struct irq_work {
	int a;
};

int __thread my_smp_processor_id;

#define raw_smp_processor_id() my_smp_processor_id
#define smp_processor_id() my_smp_processor_id

#define WARN_ON_ONCE(c) ({ int __c = (c);  if (__c) abort(); c; })

volatile unsigned long jiffies = 0;

typedef int raw_spinlock_t;
#define ____cacheline_internodealigned_in_smp

struct list_head {
	struct list_head *prev;
	struct list_head *next;
};

#define __percpu
#define __init
#define __cpuinit

//MOVED HERE
#include "atomic.h"

typedef atomic_t atomic_long_t;
typedef int wait_queue_head_t;

struct mutex {
	int a;
};
struct completion {
	int a;
};

#define DECLARE_PER_CPU(t, n) t

#define cpu_is_offline(c) 0

#define for_each_possible_cpu(cpu) for ((cpu) = 0; (cpu) < nr_cpu_ids; (cpu)++)

#define per_cpu_ptr(p, cpu) (&(p)[cpu])

#define DYNTICK_TASK_NEST_WIDTH 7
#define DYNTICK_TASK_NEST_VALUE ((LLONG_MAX >> DYNTICK_TASK_NEST_WIDTH) + 1)
#define DYNTICK_TASK_NEST_MASK  (LLONG_MAX - DYNTICK_TASK_NEST_VALUE + 1)
#define DYNTICK_TASK_FLAG	   ((DYNTICK_TASK_NEST_VALUE / 8) * 2)
#define DYNTICK_TASK_MASK	   ((DYNTICK_TASK_NEST_VALUE / 8) * 3)
#define DYNTICK_TASK_EXIT_IDLE	   (DYNTICK_TASK_NEST_VALUE + \
				    DYNTICK_TASK_FLAG)

