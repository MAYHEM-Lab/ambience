#pragma once

#if defined(TOS_ARCH_avr)
#define TOS_MAIN __attribute__((OS_main))
#define TOS_TASK __attribute__((OS_task))
#else
#define TOS_MAIN
#define TOS_TASK
#endif

#define ALWAYS_INLINE [[gnu::always_inline]] inline
#define NO_INLINE [[gnu::noinline]]

#define TOS_EXPORT [[gnu::visibility("default")]]
#define TOS_HIDDEN [[gnu::visibility("hidden")]]

#define WEAK [[gnu::weak]]

#define PURE [[gnu::pure]]

#define TOS_UNREACHABLE() __builtin_unreachable()

#if defined(__GNUC__) && !defined(__clang__)
#define TOS_NO_OPTIMIZE [[gnu::optimize("-O0")]]
#define TOS_SIZE_OPTIMIZE [[gnu::optimize("Os")]]
#elif defined(__clang__)
#define TOS_NO_OPTIMIZE [[clang::optnone]]
#define TOS_SIZE_OPTIMIZE [[clang::minsize]]
#endif