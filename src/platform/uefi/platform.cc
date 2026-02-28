/**
 * platform.cc - UEFI Platform Abstraction Layer Core
 *
 * Provides ExitProcess implementation for UEFI.
 */

#include "platform.h"
#include "efi_context.h"

// QEMU debug exit port (configured with -device isa-debug-exit,iobase=0xf4,iosize=0x04)
// QEMU maps: host_exit = (value << 1) | 1 (always non-zero)
#define QEMU_DEBUG_EXIT_PORT 0xf4

/**
 * QemuDebugExit - Signal exit code to QEMU
 *
 * On x86: Uses isa-debug-exit port, but only for failure (non-zero codes).
 * isa-debug-exit maps host_exit = (value << 1) | 1, which is always non-zero.
 * For success (code 0), we skip the port write and let ResetSystem perform
 * an ACPI shutdown so QEMU exits with 0 — matching aarch64 semihosting behavior.
 *
 * On aarch64: Uses semihosting SYS_EXIT which passes the exit code directly.
 *
 * @param code - Exit code to pass to QEMU
 */
static VOID QemuDebugExit(UINT32 code)
{
#if defined(__x86_64__) || defined(__i386__)
	if (code != 0)
	{
		__asm__ volatile("outb %0, %1" : : "a"((UINT8)code), "Nd"((UINT16)QEMU_DEBUG_EXIT_PORT));
	}
#elif defined(__aarch64__)
	// aarch64: Use semihosting to exit
	// SYS_EXIT (0x18) with ADP_Stopped_ApplicationExit (0x20026)
	// Register X0 = 0x18 (SYS_EXIT), X1 = pointer to parameter block
	UINT64 params[2] = {0x20026, code}; // ADP_Stopped_ApplicationExit, exit code
	__asm__ volatile("mov x0, #0x18\n"	// SYS_EXIT
					 "mov x1, %0\n"		// parameter block
					 "hlt #0xf000"		// semihosting call
					 :
					 : "r"(params)
					 : "x0", "x1");
#endif
}

/**
 * ExitProcess - Shutdown the system
 *
 * Signals exit code to QEMU via debug port, then uses EFI Runtime
 * Services ResetSystem() to power off the machine.
 *
 * @param code - Exit code (0 = success, non-zero = error)
 */
NO_RETURN VOID ExitProcess(USIZE code)
{
	EFI_CONTEXT *ctx = GetEfiContext();
	EFI_RUNTIME_SERVICES *rs = ctx->SystemTable->RuntimeServices;

	// Signal exit code to QEMU before shutdown
	QemuDebugExit((UINT32)code);

	// Shutdown the system
	rs->ResetSystem(EfiResetShutdown, (EFI_STATUS)code, 0, nullptr);

	// Should never reach here
	__builtin_unreachable();
}
