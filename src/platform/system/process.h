/**
 * @file process.h
 * @brief Process execution functions
 * @details Provides process creation via fork/exec.
 * Position-independent with no data section dependencies.
 * Part of the PLATFORM layer of the Position-Independent Runtime (PIR).
 */

#pragma once

#include "core/core.h"

/**
 * Process - Static class for process management operations
 *
 * Provides fork/exec functionality for process creation.
 */
class Process
{
public:
	/**
	 * Fork - Create a child process
	 *
	 * @return Result with 0 in child, child PID in parent, Error on failure
	 */
	[[nodiscard]] static Result<SSIZE, Error> Fork() noexcept;

	/**
	 * Dup2 - Duplicate file descriptor
	 *
	 * @param oldfd Source file descriptor
	 * @param newfd Target file descriptor
	 * @return Result with newfd on success, Error on failure
	 */
	[[nodiscard]] static Result<SSIZE, Error> Dup2(SSIZE oldfd, SSIZE newfd) noexcept;

	/**
	 * Execve - Execute a program
	 *
	 * @param pathname Path to executable
	 * @param argv Argument array (nullptr terminated)
	 * @param envp Environment array (nullptr terminated)
	 * @return Does not return on success, Error on failure
	 */
	[[nodiscard]] static Result<SSIZE, Error> Execve(const CHAR *pathname, CHAR *const argv[], CHAR *const envp[]) noexcept;

	/**
	 * Setsid - Create a new session
	 *
	 * @return Result with session ID on success, Error on failure
	 */
	[[nodiscard]] static Result<SSIZE, Error> Setsid() noexcept;

	// Prevent instantiation
	VOID *operator new(USIZE) = delete;
	VOID operator delete(VOID *) = delete;
	VOID *operator new(USIZE, PVOID ptr) noexcept { return ptr; }
	VOID operator delete(VOID *, PVOID) noexcept {}
};
