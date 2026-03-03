/**
 * environment.cc - FreeBSD Environment Variable Implementation
 *
 * FreeBSD does not have /proc/self/environ by default (procfs is optional).
 * This is a stub implementation that returns 0 (not found) for all variables.
 * Future enhancement: use sysctl(kern.proc.env) to read process environment.
 */

#include "platform/system/environment.h"

USIZE Environment::GetVariable(const CHAR* name, Span<CHAR> buffer) noexcept
{
	if (name == nullptr || buffer.Size() == 0)
	{
		return 0;
	}

	// FreeBSD has optional procfs. Environment variables are not accessible
	// via a simple file read in freestanding mode.
	// Return empty result.
	buffer[0] = '\0';
	return 0;
}
