#pragma once

// macOS BSD syscall wrappers — architecture-specific implementations
#if defined(ARCHITECTURE_X86_64)
#include "platform/os/macos/common/system.x86_64.h"
#elif defined(ARCHITECTURE_AARCH64)
#include "platform/os/macos/common/system.aarch64.h"
#else
#error "Unsupported architecture"
#endif
