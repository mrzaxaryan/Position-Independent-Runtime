#include "platform/platform.h"
#include "platform/os/linux/common/syscall.h"
#include "platform/os/linux/common/system.h"

// Linux process exit implementation
NO_RETURN VOID ExitProcess(USIZE code)
{
	System::Call(SYS_EXIT, code);
	__builtin_unreachable();
}
