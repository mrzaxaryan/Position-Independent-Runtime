#include "random.h"
#include "DateTime.h"
#include "platform.h"

// Function to get a random number in the range of 0 to RANDOM_MAX
INT32 Random::Get()
{
    // Update seed using linear congruential generator
    this->seed = (this->seed * 214013L + 2531011L) & 0x7fffffff;
    // Return value in range [0, Random::MAX) using bit shift
    // Random::MAX is 32767 (0x7FFF), so we can use the upper 15 bits
    return (INT32)((this->seed >> 16) & 0x7FFF);
}

// Constructor to initialize the random number generator
Random::Random()
{
    // TODO: For now use a fixed seed - will implement time-based seeding later
    // The DateTime::Now() call appears to be causing issues in the test environment
    this->seed = 12345;  // Fixed seed for reproducible testing
}
