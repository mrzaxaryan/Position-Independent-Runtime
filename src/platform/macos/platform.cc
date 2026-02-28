#include "platform.h"
#include "syscall.h"
#include "system.h"

#if defined(ARCHITECTURE_AARCH64)
// ARM64 macOS cannot use -static (kernel requires dyld). The linker adds
// dyld_stub_binder to the initial undefined symbols list for all dynamic
// executables. Normally libSystem provides it, but -nostdlib prevents linking
// libSystem. This no-op stub satisfies the linker. It is never called because
// -fvisibility=hidden eliminates all lazy-binding stubs.
extern "C" void dyld_stub_binder() {}
#endif

// macOS process exit implementation
NO_RETURN VOID ExitProcess(USIZE code)
{
	System::Call(SYS_EXIT, code);
	__builtin_unreachable();
}
