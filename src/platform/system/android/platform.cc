#include "platform/platform.h"
#include "platform/common/android/syscall.h"
#include "platform/common/android/system.h"

// Android process exit implementation
NO_RETURN VOID ExitProcess(USIZE code)
{
	System::Call(SYS_EXIT, code);
	__builtin_unreachable();
}
