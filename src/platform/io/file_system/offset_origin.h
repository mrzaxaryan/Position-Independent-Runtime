#pragma once

#include "primitives.h"

// Offset origin for file seeking
enum class OffsetOrigin : INT32
{
	Start = 0,   // Beginning of the file
	Current = 1, // Current file pointer position
	End = 2      // End of the file
};
