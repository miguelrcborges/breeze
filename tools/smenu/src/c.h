#ifndef C_H

#include <stdint.h>

enum CONSTANTS {
	DEFAULT_BACKGROUND_COLOR  = 0x282828, // ggrrbb
	DEFAULT_BACKGROUND_COLOR2 = 0x2f3032, 
	DEFAULT_FOREGROUND_COLOR  = 0x98bed4, 

	DEFAULT_FONT_SIZE = 24,

	MAIN_WINDOW_PADDING = 16,
	SEARCH_BOX_WINDOW_PADDING = 8,
};

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

#endif /* C_H */
