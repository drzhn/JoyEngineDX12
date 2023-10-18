#ifndef HASHDEFS_H
#define HASHDEFS_H

#include <cstdint>
#include <random>

constexpr uint64_t val_64_const = 0xcbf29ce484222325;
constexpr uint64_t prime_64_const = 0x100000001b3;

inline std::uniform_int_distribution<uint64_t> generator(0ull);
inline std::mt19937 gen(1729ull);

constexpr uint32_t StrHash32(const char* s, uint32_t hash = 5381) noexcept
{
	return !*s ? hash : StrHash32(s + 1, uint32_t(hash * uint64_t(33) ^ *s));
}

constexpr uint64_t StrHash64(const char* str, const uint64_t value = val_64_const) noexcept
{
	return !*str ? value : StrHash64(&str[1], (value ^ static_cast<uint64_t>(static_cast<uint8_t>(str[0]))) * prime_64_const);
}

inline uint64_t RandomHash64()
{
	return generator(gen);
}

#define HASH(T) StrHash32(#T)

#endif // HASHDEFS_H
