/**
 * process.cc - Windows Process Execution Implementation
 *
 * Windows does not support POSIX fork/dup2/execve/setsid.
 * All functions return failure.
 */

#include "platform/system/process.h"

// Windows doesn't have fork() - use stub implementation
Result<SSIZE, Error> Process::Fork() noexcept
{
	return Result<SSIZE, Error>::Err(Error::Process_ForkFailed);
}

// Windows doesn't have dup2 - stub
Result<SSIZE, Error> Process::Dup2(SSIZE oldfd, SSIZE newfd) noexcept
{
	(void)oldfd;
	(void)newfd;
	return Result<SSIZE, Error>::Err(Error::Process_Dup2Failed);
}

// Windows doesn't have execve - stub
Result<SSIZE, Error> Process::Execve(const CHAR *pathname, CHAR *const argv[], CHAR *const envp[]) noexcept
{
	(void)pathname;
	(void)argv;
	(void)envp;
	return Result<SSIZE, Error>::Err(Error::Process_ExecveFailed);
}

// Windows doesn't have setsid - stub
Result<SSIZE, Error> Process::Setsid() noexcept
{
	return Result<SSIZE, Error>::Err(Error::Process_SetsidFailed);
}
