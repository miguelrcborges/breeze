#ifndef MBBASE_H
#define MBBASE_H

#ifdef __cplusplus
extern "C" {
#endif

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
typedef uintptr_t uptr;
typedef ptrdiff_t sptr;
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
	#define force_inline __attribute__((always_inline)) inline
	#define no_inline __attribute__((noinline))
#elif defined(_MSC_VER)
	#define force_inline __forceinline inline
	#define no_inline __declspec(noinline)
#else
	#define force_inline inline
	#define no_inline
#endif

#if __has_builtin(__builtin_expect)
	#define likely(expr) __builtin_expect(!!(expr), 1)
	#define unlikely(expr) __builtin_expect(!!(expr), 0)
#else 
	#define likely(expr) (expr)
	#define unlikely(expr) (expr)
#endif

#define len(a) (sizeof(a)/sizeof(*(a)))
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define clamp(a,x,b) (((x)<(a))?(a):((x)>(b))?(b):(x))


#ifdef __cplusplus
}
#endif

#endif /* MBBASE_H */
