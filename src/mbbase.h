#ifndef MBBASE_H

#include <stdint.h>
#include <stddef.h>

typedef int_least8_t i8;
typedef int_least16_t i16;
typedef int_least32_t i32;
typedef int_least64_t i64;

typedef uint_least8_t u8;
typedef uint_least16_t u16;
typedef uint_least32_t u32;
typedef uint_least64_t u64;

typedef float f32;
typedef double f64;

typedef size_t usize;
typedef ptrdiff_t isize;
typedef uintptr_t uptr;

#if !defined(bool) && !defined(__cplusplus) && __STDC_VERSION__ <= 201710L
typedef u8 bool;
enum {
	false = 0,
	true = 1
};
#endif


#define STATEMENT(S) do{S}while(0)
#define _STRINGIFY(S) #S
#define STRINGIFY(S) _STRINGIFY(S)
#define _GLUE(A,B) A##B
#define GLUE(A,B) _GLUE

#ifndef __has_builtin
	#define __has_builtin(X) (0)
#endif

#ifndef __has_feature
  #define __has_feature(x) (0)
#endif

#if defined(__GNUC__)
	#define force_inline static __attribute__((always_inline)) inline
	#define no_inline __attribute__((noinline))
#elif defined(_MSC_VER)
	#define force_inline static __forceinline
	#define no_inline __declspec(noinline)
#else
	#define force_inline static inline
	#define no_inline
#endif

#if __has_builtin(__builtin_expect)
	#define likely(expr) __builtin_expect(!!(expr), 1)
	#define unlikely(expr) __builtin_expect(!!(expr), 0)
#else 
	#define likely(expr) (expr)
	#define unlikely(expr) (expr)
#endif

#if !__has_builtin(__builtin_unreachable)
	#ifdef _MSC_VER
		#define __builtin_unreachable() __assume(0)	
	#else
		#define __builtin_unreachable()
	#endif
#endif

#if !__has_builtin(__builtin_trap)
	#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
		#define __builtin_trap() __debugbreak()
	#else
		#define __builtin_trap() (*(int *)0)
	#endif
#endif
		

#define size(x) ((isize) sizeof(x))
#define len(a) (size(a)/size(*(a)))
//#define min(a,b) (((a)<(b))?(a):(b))
//#define max(a,b) (((a)>(b))?(a):(b))
#define clamp(a,x,b) (((x)<(a))?(a):((x)>(b))?(b):(x))


#define MBBASE_H
#endif
