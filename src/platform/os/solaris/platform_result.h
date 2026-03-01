#pragma once

#include "core/types/result.h"
#include "core/types/error.h"

namespace result
{

/// Solaris syscall: success when result >= 0, failure stores -result as errno.
/// The carry-flag negation happens in system.h, so by the time we reach here,
/// negative return = error (same convention as FromLinux/FromMacOS).
template <typename T>
[[nodiscard]] FORCE_INLINE Result<T, Error> FromSolaris(SSIZE result) noexcept
{
	if (result >= 0)
	{
		if constexpr (__is_same_as(T, void))
			return Result<T, Error>::Ok();
		else
			return Result<T, Error>::Ok((T)result);
	}
	return Result<T, Error>::Err(Error::Posix((UINT32)(-result)));
}

} // namespace result
