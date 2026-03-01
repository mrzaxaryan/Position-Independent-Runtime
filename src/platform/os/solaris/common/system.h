#pragma once

// Solaris/illumos syscall wrappers — architecture-specific implementations
#if defined(ARCHITECTURE_X86_64)
#include "platform/os/solaris/common/system.x86_64.h"
#elif defined(ARCHITECTURE_I386)
#include "platform/os/solaris/common/system.i386.h"
#elif defined(ARCHITECTURE_AARCH64)
#include "platform/os/solaris/common/system.aarch64.h"
#else
#error "Unsupported architecture"
#endif
