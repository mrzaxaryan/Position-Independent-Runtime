#if defined(PLATFORM_LINUX)
#pragma once 
#include "platform.h"

/* -------------------------------------------------------------------------- */
/* POSIX/Linux Constants                                                       */
/* -------------------------------------------------------------------------- */

/* Standard file descriptors */
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* Memory protection flags */
#define PROT_READ    0x01
#define PROT_WRITE   0x02

/* Memory mapping flags */
#define MAP_PRIVATE    0x02
#define MAP_ANONYMOUS  0x20
#define MAP_FAILED     ((PVOID)(-1))

class Syscall
{
public:
    /* File I/O */
    static SSIZE Write(INT32 fd, PCVOID buf, USIZE count);

    /* Process operations */
    static SSIZE Exit(INT32 status);

    /* Memory operations */
    static PVOID Mmap(PVOID addr, USIZE length, INT32 prot, INT32 flags, INT32 fd, SSIZE offset);
    static SSIZE Munmap(PVOID addr, USIZE length);
};

#endif // PLATFORM_LINUX
