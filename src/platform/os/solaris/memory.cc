#include "platform/memory/allocator.h"
#include "platform/os/solaris/common/syscall.h"
#include "platform/os/solaris/common/system.h"

// Memory allocator using mmap/munmap (Solaris syscalls)
// Same pattern as Linux/macOS but with Solaris MAP_ANONYMOUS = 0x100

PVOID Allocator::AllocateMemory(USIZE size)
{
	if (size == 0)
		return nullptr;

	// Align size to page boundary (4096 bytes)
	size = (size + 4095) & ~4095ULL;

	PVOID addr = nullptr;
	INT32 prot = PROT_READ | PROT_WRITE;
	INT32 flags = MAP_PRIVATE | MAP_ANONYMOUS;

	SSIZE result = System::Call(SYS_MMAP, (USIZE)addr, size, prot, flags, -1, (USIZE)0);

	if (result == -1 || result < 0)
		return nullptr;

	return (PVOID)result;
}

VOID Allocator::ReleaseMemory(PVOID address, USIZE size)
{
	if (address == nullptr || size == 0)
		return;

	size = (size + 4095) & ~4095ULL;
	System::Call(SYS_MUNMAP, (USIZE)address, size);
}
