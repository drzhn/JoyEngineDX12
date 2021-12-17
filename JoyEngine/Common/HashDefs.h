#ifndef HASHDEFS_H
#define HASHDEFS_H

#include <cstdint>

constexpr const uint32_t strHash(const char* s, uint32_t hash = 5381) noexcept
{
	return !*s ? hash : strHash(s + 1, uint32_t(hash * uint64_t(33) ^ *s));
}

#define HASH(T) strHash(#T)

#endif // HASHDEFS_H
