#pragma once
#include "primitives.h"

// DJB2 hash function implementation
consteval USIZE ct_hash_str_seed(const CHAR *s)
{
    USIZE h = (USIZE)2166136261u;
    for (USIZE i = 0; s[i] != '\0'; ++i)
        h = (h ^ (USIZE)(UINT8)s[i]) * (USIZE)16777619u;
    return h;
}

// Class to handle DJB2 hashing
class Djb2
{
private:
    static constexpr USIZE Seed = ct_hash_str_seed(__DATE__ " " __TIME__); // Seed based on compile date and time

public:
    // Hash function for strings (case-insensitive)
    template <typename TChar>
    static USIZE Hash(const TChar *value)
    {
        USIZE h = Seed;
        for (USIZE i = 0; value[i] != (TChar)0; ++i)
        {
            TChar c = value[i];
            if (c >= (TChar)'A' && c <= (TChar)'Z')
                c += (TChar)('a' - 'A');
            h = ((h << 5) + h) + (USIZE)c;
        }
        return h;
    }

    // Compile-time hash function for string literals (case-insensitive)
    template <typename TChar, USIZE N>
    static consteval USIZE HashCompileTime(const TChar (&value)[N])
    {
        USIZE h = Seed;
        for (USIZE i = 0; i + 1 < N; ++i)
        {
            TChar c = value[i];
            if (c >= (TChar)'A' && c <= (TChar)'Z')
                c += (TChar)('a' - 'A');
            h = ((h << 5) + h) + (USIZE)c;
        }
        return h;
    }
};
