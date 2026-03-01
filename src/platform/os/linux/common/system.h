#pragma once

// Linux syscall wrappers — architecture-specific implementations
#if defined(ARCHITECTURE_X86_64)
#include "platform/os/linux/common/system.x86_64.h"
#elif defined(ARCHITECTURE_I386)
#include "platform/os/linux/common/system.i386.h"
#elif defined(ARCHITECTURE_AARCH64)
#include "platform/os/linux/common/system.aarch64.h"
#elif defined(ARCHITECTURE_ARMV7A)
#include "platform/os/linux/common/system.armv7a.h"
#else
#error "Unsupported architecture"
#endif
