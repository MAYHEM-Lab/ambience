//
// Created by fatih on 5/29/18.
//

#pragma once

#define TOS_MAIN __attribute__((OS_main))
#define NORETURN __attribute__((noreturn))
#define TOS_TASK __attribute__((OS_task))
#define ALWAYS_INLINE __attribute__((always_inline))

#define TOS_EXPORT __attribute__((visibility("default")))