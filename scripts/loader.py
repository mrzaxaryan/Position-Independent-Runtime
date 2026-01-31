#!/usr/bin/env python3
"""
PIC Shellcode Loader

Loads position-independent code into executable memory and runs it.
Auto-detects the current platform.

Usage:
    python loader.py --arch x86_64 shellcode.bin
    python loader.py --arch i386 shellcode.bin
"""

import argparse
import ctypes
import mmap
import platform
import struct
import sys

# Entry offsets match cpp-pic linker script
ARCH = {
    'i386':    {'bits': 32, 'family': 'x86', 'entry': 0x70},
    'x86_64':  {'bits': 64, 'family': 'x86', 'entry': 0x1e0},
    'aarch64': {'bits': 64, 'family': 'arm', 'entry': 0x78},
}


def get_host():
    """Detect host OS, CPU family, and bitness."""
    os_name = platform.system().lower()
    machine = platform.machine().lower()

    if machine in ('amd64', 'x86_64'):
        return os_name, 'x86', 64
    elif machine in ('arm64', 'aarch64'):
        return os_name, 'arm', 64
    elif machine in ('i386', 'i686', 'x86'):
        return os_name, 'x86', 32
    return os_name, machine, 64


def run_mmap(shellcode, entry_offset):
    """Execute via mmap (Linux/macOS)."""
    mem = mmap.mmap(-1, len(shellcode), prot=mmap.PROT_READ | mmap.PROT_WRITE | mmap.PROT_EXEC)
    mem.write(shellcode)

    base = ctypes.addressof(ctypes.c_char.from_buffer(mem))
    entry = base + entry_offset

    print(f"[+] Base: 0x{base:x}, Entry: 0x{entry:x}")
    print("[*] Executing...")
    sys.stdout.flush()

    return ctypes.CFUNCTYPE(ctypes.c_int)(entry)()


def run_virtualalloc(shellcode, entry_offset):
    """Execute via VirtualAlloc (Windows)."""
    from ctypes import wintypes

    k32 = ctypes.windll.kernel32
    ptr = k32.VirtualAlloc(None, len(shellcode), 0x3000, 0x40)
    if not ptr:
        raise OSError("VirtualAlloc failed")

    ctypes.memmove(ptr, shellcode, len(shellcode))
    entry = ptr + entry_offset

    print(f"[+] Base: 0x{ptr:x}, Entry: 0x{entry:x}")
    print("[*] Executing...")
    sys.stdout.flush()

    thread = k32.CreateThread(None, 0, entry, None, 0, None)
    if not thread:
        raise OSError("CreateThread failed")

    k32.WaitForSingleObject(thread, 0xFFFFFFFF)
    code = wintypes.DWORD()
    k32.GetExitCodeThread(thread, ctypes.byref(code))
    k32.CloseHandle(thread)
    return code.value


def main():
    parser = argparse.ArgumentParser(description='PIC Shellcode Loader')
    parser.add_argument('--arch', required=True, choices=['i386', 'x86_64', 'aarch64'])
    parser.add_argument('shellcode')
    args = parser.parse_args()

    host_os, host_family, host_bits = get_host()
    target = ARCH[args.arch]

    print(f"[*] Host: {host_os}/{host_family}/{host_bits}bit")
    print(f"[*] Target: {args.arch}")

    if target['family'] != host_family:
        sys.exit(f"[-] Cannot run {target['family']} on {host_family}")

    if host_bits != target['bits']:
        sys.exit(f"[-] Bitness mismatch: host={host_bits}bit, target={target['bits']}bit")

    with open(args.shellcode, 'rb') as f:
        shellcode = f.read()
    print(f"[+] Loaded: {len(shellcode)} bytes")

    if host_os == 'windows':
        code = run_virtualalloc(shellcode, target['entry'])
    else:
        code = run_mmap(shellcode, target['entry'])

    print(f"[+] Exit: {code}")
    sys.exit(code)


if __name__ == '__main__':
    main()
