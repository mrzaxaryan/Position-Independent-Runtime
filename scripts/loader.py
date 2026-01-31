#!/usr/bin/env python3
"""
PIC Shellcode Loader

Loads position-independent code into executable memory and runs it.
Auto-detects platform. Reads entry point from corresponding .elf file.

Usage:
    python loader.py --arch x86_64 output.bin
    python loader.py --arch i386 output.bin
"""

import argparse
import ctypes
import mmap
import os
import platform
import struct
import sys

ARCH = {
    'i386':    {'bits': 32, 'family': 'x86'},
    'x86_64':  {'bits': 64, 'family': 'x86'},
    'aarch64': {'bits': 64, 'family': 'arm'},
}


def get_host():
    os_name = platform.system().lower()
    machine = platform.machine().lower()

    if machine in ('amd64', 'x86_64'):
        return os_name, 'x86', 64
    elif machine in ('arm64', 'aarch64'):
        return os_name, 'arm', 64
    elif machine in ('i386', 'i686', 'x86'):
        return os_name, 'x86', 32
    return os_name, machine, 64


def read_elf_entry(path):
    """Read entry point from ELF file."""
    with open(path, 'rb') as f:
        if f.read(4) != b'\x7fELF':
            return None
        ei_class = struct.unpack('B', f.read(1))[0]
        f.seek(0x18)
        if ei_class == 1:  # 32-bit
            return struct.unpack('<I', f.read(4))[0]
        else:  # 64-bit
            return struct.unpack('<Q', f.read(8))[0]


def get_entry_offset(bin_path):
    """Get entry offset from .elf file next to .bin."""
    elf_path = bin_path.rsplit('.', 1)[0] + '.elf'
    if os.path.exists(elf_path):
        entry = read_elf_entry(elf_path)
        if entry:
            return entry
    # Try .exe for Windows builds
    exe_path = bin_path.rsplit('.', 1)[0] + '.exe'
    if os.path.exists(exe_path):
        # PE entry point reading would go here
        pass
    sys.exit(f"[-] Cannot find .elf file to read entry point: {elf_path}")


def run_mmap(shellcode, entry_offset):
    mem = mmap.mmap(-1, len(shellcode), prot=mmap.PROT_READ | mmap.PROT_WRITE | mmap.PROT_EXEC)
    mem.write(shellcode)

    base = ctypes.addressof(ctypes.c_char.from_buffer(mem))
    entry = base + entry_offset

    print(f"[+] Base: 0x{base:x}, Entry: 0x{entry:x}")
    print("[*] Executing...")
    sys.stdout.flush()

    return ctypes.CFUNCTYPE(ctypes.c_int)(entry)()


def run_virtualalloc(shellcode, entry_offset):
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

    entry_offset = get_entry_offset(args.shellcode)
    print(f"[+] Entry offset: 0x{entry_offset:x}")

    with open(args.shellcode, 'rb') as f:
        shellcode = f.read()
    print(f"[+] Loaded: {len(shellcode)} bytes")

    if host_os == 'windows':
        code = run_virtualalloc(shellcode, entry_offset)
    else:
        code = run_mmap(shellcode, entry_offset)

    print(f"[+] Exit: {code}")
    sys.exit(code)


if __name__ == '__main__':
    main()
