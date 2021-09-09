#pragma once

#if defined(TOS_PLATFORM_avr)
#define TOS_MAIN __attribute__((OS_main))
#define TOS_TASK __attribute__((OS_task))
#else
#define TOS_MAIN
#define TOS_TASK
#endif

#if defined(PACKED)
#undef PACKED
#endif

#if defined(__GNUC__) || defined(__clang__)
#define ALWAYS_INLINE     [[gnu::always_inline]] inline
#define NO_INLINE         [[gnu::noinline]]
#define WEAK              [[gnu::weak]]
#define USED              [[gnu::used]]
#define PACKED            [[gnu::packed]]
#define PURE              [[gnu::pure]]
#define TOS_UNREACHABLE() __builtin_unreachable()
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define TOS_NO_OPTIMIZE   [[gnu::optimize("-O0")]]
#define TOS_SIZE_OPTIMIZE [[gnu::optimize("Os")]]
#define TOS_GCC
#elif defined(__clang__)
#define TOS_NO_OPTIMIZE   [[clang::optnone]]
#define TOS_SIZE_OPTIMIZE [[clang::minsize]]
#define TOS_CLANG
#elif defined(_MSC_VER)
#define ALWAYS_INLINE __forceinline
#define NO_INLINE __declspec(noinline)
#define TOS_NO_OPTIMIZE
#define TOS_SIZE_OPTIMIZE
#define WEAK
#define USED
#define PACKED
#define PURE
#define TOS_UNREACHABLE() __assume(0)
#define TOS_MSVC
#endif

#define ISR_AVAILABLE

// Used to mark a trivially constructable global variable not to be zero initialized.
// Using such an object without prior initialization is UB!
#define NO_ZERO [[gnu::section(".nozero")]]
