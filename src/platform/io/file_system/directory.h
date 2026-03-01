#pragma once

#include "core/types/primitives.h"
#include "core/types/error.h"
#include "core/types/result.h"

// Class for directory operations
class Directory
{
public:
	[[nodiscard]] static Result<void, Error> Create(PCWCHAR path);
	[[nodiscard]] static Result<void, Error> Delete(PCWCHAR path);
};
