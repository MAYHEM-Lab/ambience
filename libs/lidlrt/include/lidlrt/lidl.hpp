#pragma once

#include "array.hpp"
#include "enumeration.hpp"
#include "lib.hpp"
#include "ptr.hpp"
#include "string.hpp"
#include "structure.hpp"
#include "union.hpp"
#include "vector.hpp"
#include <lidlrt/find_extent.hpp>
#include <tos/fixed_string.hpp>

#ifndef LIDL_ASSERT
#ifdef TOS
#include <tos/debug/assert.hpp>
#define LIDL_ASSERT Assert
#else
#include <cassert>
#define LIDL_ASSERT assert
#endif
#endif

#ifndef LIDL_UNION_ASSERT
#define LIDL_UNION_ASSERT LIDL_ASSERT
#endif

#ifndef LIDL_UNREACHABLE
#ifdef TOS
#include <tos/compiler.hpp>
#define LIDL_UNREACHABLE() TOS_UNREACHABLE()
#else
#if defined(__GNUC__) && !defined(__clang__)
#define LIDL_UNREACHABLE() __builtin_unreachable()
#else
#define LIDL_UNREACHABLE() LIDL_ASSERT(false)
#endif
#endif
#endif