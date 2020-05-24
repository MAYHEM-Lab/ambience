#include <tos/debug/assert.hpp>

#define STBTT_assert(x) Assert(x)
#define STBTT_malloc(x, u) new char[x]
#define STBTT_free(x, u) delete[] reinterpret_cast<char*>(x)

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"