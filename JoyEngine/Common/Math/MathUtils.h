#pragma once

#include <concepts>
#include <cstdint>

namespace JoyEngine
{
	namespace jmath
	{
		template <typename T> requires std::integral<T> || std::floating_point<T>
		T align(T size, T alignment)
		{
			return ((size - 1) / alignment + 1) * alignment;
		}
	}
}
