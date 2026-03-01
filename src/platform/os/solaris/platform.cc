#include "platform/platform.h"
#include "platform/os/solaris/common/syscall.h"
#include "platform/os/solaris/common/system.h"

// Solaris process exit implementation
NO_RETURN VOID ExitProcess(USIZE code)
{
	System::Call(SYS_EXIT, code);
	__builtin_unreachable();
}
