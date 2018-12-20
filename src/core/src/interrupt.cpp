#include <tos/interrupt.hpp>

namespace tos
{
	namespace kern
	{
		int8_t detail::disable_depth = 1;
	}
}