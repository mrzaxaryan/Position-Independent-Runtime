#if defined(PLATFORM_LINUX)

#include "linux/syscall.h"

NO_RETURN VOID ExitProcess(USIZE code)
{
    Syscall::Exit((INT32)code);
    __builtin_unreachable();
}

#endif