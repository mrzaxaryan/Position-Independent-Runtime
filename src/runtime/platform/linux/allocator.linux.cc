#if defined(PLATFORM_LINUX)

#include "allocator.h"
#include "linux/syscall.h"

typedef struct _ALLOC_HDR
{
    USIZE Size; // user requested bytes
} ALLOC_HDR;

static inline USIZE __align_up(USIZE x, USIZE a)
{
    return (x + (a - 1)) & ~(a - 1);
}

PVOID Allocator::AllocateMemory(USIZE len)
{
    const USIZE page = 4096;
    const USIZE total = __align_up((USIZE)sizeof(ALLOC_HDR) + len, page);

    PVOID base = Syscall::Mmap(
        NULL,
        total,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);

    if (base == MAP_FAILED)
        return (PVOID)0;

    ALLOC_HDR *h = (ALLOC_HDR *)base;
    h->Size = len;
    return (PVOID)(h + 1);
}

VOID Allocator::ReleaseMemory(PVOID ptr, USIZE sizeHint)
{
    if (!ptr)
        return;

    ALLOC_HDR *h = ((ALLOC_HDR *)ptr) - 1;
    const USIZE realSize = h->Size;

#if defined(DEBUG)
    // If caller gave size, validate it. If sizeHint==0, unsized delete path.
    if (sizeHint != 0 && sizeHint != realSize)
        __builtin_trap(); // or your ASSERT/Abort
#else
    (VOID)sizeHint;
#endif

    const USIZE page = 4096;
    const USIZE total = __align_up((USIZE)sizeof(ALLOC_HDR) + realSize, page);

    Syscall::Munmap(h, total);
}

#endif
