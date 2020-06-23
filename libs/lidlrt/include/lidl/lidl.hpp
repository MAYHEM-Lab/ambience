#pragma once

#include "array.hpp"
#include "enumeration.hpp"
#include "lib.hpp"
#include "ptr.hpp"
#include "string.hpp"
#include "structure.hpp"
#include "union.hpp"
#include "vector.hpp"

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
