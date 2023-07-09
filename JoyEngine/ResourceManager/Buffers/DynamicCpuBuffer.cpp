#include "DynamicCpuBuffer.h"

namespace JoyEngine
{
	uint32_t Align(uint32_t size, uint32_t alignment)
	{
		return ((size - 1) / alignment + 1) * alignment;
	}
}
