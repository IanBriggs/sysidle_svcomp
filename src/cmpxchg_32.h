#ifndef _ASM_X86_CMPXCHG_32_H
#define _ASM_X86_CMPXCHG_32_H

/*
 * Note: if you use set64_bit(), __cmpxchg64(), or their variants, you
 *       you need to test for the feature in boot_cpu_data.
 */

/*
 * CMPXCHG8B only writes to the target if we had the previous
 * value in registers, otherwise it acts as a read and gives us the
 * "new previous" value.  That is why there is a loop.  Preloading
 * EDX:EAX is a performance optimization: in the common case it means
 * we need only one locked operation.
 *
 * A SIMD/3DNOW!/MMX/FPU 64-bit store here would require at the very
 * least an FPU save and/or %cr0.ts manipulation.
 *
 * cmpxchg8b must be used with the lock prefix here to allow the
 * instruction to be executed atomically.  We need to have the reader
 * side to see the coherent 64bit value.
 */
/* UNUSED static inline void set_64bit(volatile u64 *ptr, u64 value) */
/* UNUSED { */
/* UNUSED 	u32 low  = value; */
/* UNUSED 	u32 high = value >> 32; */
/* UNUSED 	u64 prev = *ptr; */

/* UNUSED 	asm volatile("\n1:\t" */
/* UNUSED 		     LOCK_PREFIX "cmpxchg8b %0\n\t" */
/* UNUSED 		     "jnz 1b" */
/* UNUSED 		     : "=m" (*ptr), "+A" (prev) */
/* UNUSED 		     : "b" (low), "c" (high) */
/* UNUSED 		     : "memory"); */
/* UNUSED } */

#define __HAVE_ARCH_CMPXCHG 1

/* UNUSED #ifdef CONFIG_X86_CMPXCHG64 */
/* UNUSED #define cmpxchg64(ptr, o, n)						\ */
/* UNUSED 	((__typeof__(*(ptr)))__cmpxchg64((ptr), (unsigned long long)(o), \ */
/* UNUSED 					 (unsigned long long)(n))) */
/* UNUSED #define cmpxchg64_local(ptr, o, n)					\ */
/* UNUSED 	((__typeof__(*(ptr)))__cmpxchg64_local((ptr), (unsigned long long)(o), \ */
/* UNUSED 					       (unsigned long long)(n))) */
/* UNUSED #endif */

/* UNUSED static inline u64 __cmpxchg64(volatile u64 *ptr, u64 old, u64 new) */
/* UNUSED { */
/* UNUSED 	u64 prev; */
/* UNUSED 	asm volatile(LOCK_PREFIX "cmpxchg8b %1" */
/* UNUSED 		     : "=A" (prev), */
/* UNUSED 		       "+m" (*ptr) */
/* UNUSED 		     : "b" ((u32)new), */
/* UNUSED 		       "c" ((u32)(new >> 32)), */
/* UNUSED 		       "0" (old) */
/* UNUSED 		     : "memory"); */
/* UNUSED 	return prev; */
/* UNUSED } */

/* UNUSED static inline u64 __cmpxchg64_local(volatile u64 *ptr, u64 old, u64 new) */
/* UNUSED { */
/* UNUSED 	u64 prev; */
/* UNUSED 	asm volatile("cmpxchg8b %1" */
/* UNUSED 		     : "=A" (prev), */
/* UNUSED 		       "+m" (*ptr) */
/* UNUSED 		     : "b" ((u32)new), */
/* UNUSED 		       "c" ((u32)(new >> 32)), */
/* UNUSED 		       "0" (old) */
/* UNUSED 		     : "memory"); */
/* UNUSED 	return prev; */
/* UNUSED } */

#ifndef CONFIG_X86_CMPXCHG64
/*
 * Building a kernel capable running on 80386 and 80486. It may be necessary
 * to simulate the cmpxchg8b on the 80386 and 80486 CPU.
 */

/* UNUSED #define cmpxchg64(ptr, o, n)					\ */
/* UNUSED ({								\ */
/* UNUSED 	__typeof__(*(ptr)) __ret;				\ */
/* UNUSED 	__typeof__(*(ptr)) __old = (o);				\ */
/* UNUSED 	__typeof__(*(ptr)) __new = (n);				\ */
/* UNUSED 	alternative_io(LOCK_PREFIX_HERE				\ */
/* UNUSED 			"call cmpxchg8b_emu",			\ */
/* UNUSED 			"lock; cmpxchg8b (%%esi)" ,		\ */
/* UNUSED 		       X86_FEATURE_CX8,				\ */
/* UNUSED 		       "=A" (__ret),				\ */
/* UNUSED 		       "S" ((ptr)), "0" (__old),		\ */
/* UNUSED 		       "b" ((unsigned int)__new),		\ */
/* UNUSED 		       "c" ((unsigned int)(__new>>32))		\ */
/* UNUSED 		       : "memory");				\ */
/* UNUSED 	__ret; }) */


/* UNUSED #define cmpxchg64_local(ptr, o, n)				\ */
/* UNUSED ({								\ */
/* UNUSED 	__typeof__(*(ptr)) __ret;				\ */
/* UNUSED 	__typeof__(*(ptr)) __old = (o);				\ */
/* UNUSED 	__typeof__(*(ptr)) __new = (n);				\ */
/* UNUSED 	alternative_io("call cmpxchg8b_emu",			\ */
/* UNUSED 		       "cmpxchg8b (%%esi)" ,			\ */
/* UNUSED 		       X86_FEATURE_CX8,				\ */
/* UNUSED 		       "=A" (__ret),				\ */
/* UNUSED 		       "S" ((ptr)), "0" (__old),		\ */
/* UNUSED 		       "b" ((unsigned int)__new),		\ */
/* UNUSED 		       "c" ((unsigned int)(__new>>32))		\ */
/* UNUSED 		       : "memory");				\ */
/* UNUSED 	__ret; }) */ 

#endif

/* UNUSED #define system_has_cmpxchg_double() cpu_has_cx8 */

#endif /* _ASM_X86_CMPXCHG_32_H */
