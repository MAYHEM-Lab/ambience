#include <sys/types.h>

#if defined(__cplusplus)
extern "C"{
#endif

void tos_assert(const char*);
int tos_log_printf(const char* fmt, ...);

#if defined(__cplusplus)
}
#endif

#define LWIP_PLATFORM_ASSERT(x) tos_assert(x)
#define LWIP_PLATFORM_DIAG(x) tos_log_printf x
