#ifndef HASHDEFS_H
#define HASHDEFS_H

#include <cstdint>

constexpr uint64_t val_64_const = 0xcbf29ce484222325;
constexpr uint64_t prime_64_const = 0x100000001b3;

constexpr uint32_t StrHash32(const char* s, uint32_t hash = 5381) noexcept
{
	return !*s ? hash : StrHash32(s + 1, uint32_t(hash * uint64_t(33) ^ *s));
}

constexpr uint64_t StrHash64(const char* str, const uint64_t value = val_64_const) noexcept
{
	return !*str ? value : StrHash64(&str[1], (value ^ static_cast<uint64_t>(static_cast<uint8_t>(str[0]))) * prime_64_const);
}

#define HASH(T) StrHash32(#T)

#endif // HASHDEFS_H
