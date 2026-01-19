#pragma once

#include "primitives.h"

// Class to handle random number generation
class Random
{
private:
    // Internal state for the random number generator
    UINT32 seed;

public:
    VOID *operator new(USIZE) = delete; // Disable dynamic allocation
    VOID operator delete(VOID *) = delete; // Disable dynamic deallocation
    // The maximum value for the random number generator
    static constexpr INT32 MAX = 32767;
    // Constructor
    Random();

    // Generate a random string of specified length
    template <typename TChar>
    UINT32 GetString(TChar *pString, UINT32 length);
    // Generate a random character
    template <typename TChar>
    TChar GetChar();

    // Core random function
    INT32 Get();
    // Random byte array
    INT32 GetArray(USIZE size, PUINT8 buffer);
};


template <typename TChar>
TChar Random::GetChar()
{
    // Set the seed for the random number generator
    TChar c = (TChar)('a' + ((TChar)Random::Get() % 26));
    // Return the random character
    return (TChar)c;
}

template <typename TChar>
UINT32 Random::GetString(TChar *pString, UINT32 length)
{
    UINT32 i = 0; // Loop counter for the string length
    for (i = 0; i < length; i++)
    {
        pString[i] = Random::GetChar<TChar>(); // Get a random wide character
    }
    pString[length] = (TChar)'\0'; // Null-terminate the wide string
    return length;                 // Return the length of the wide string
}