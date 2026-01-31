#!/usr/bin/env python3
"""
PIC Shellcode Loader

Cross-platform loader for position-independent code.
Auto-detects platform. Uses QEMU for cross-arch on Linux.

Usage:
    python loader.py --arch x86_64 output.bin
    python loader.py --arch i386 output.bin
    python loader.py --arch aarch64 output.bin
    python loader.py --arch armv7a output.bin
"""

import argparse
import ctypes
import mmap
import os
import platform
import shutil
import struct
import subprocess
import sys

ARCH = {
    'i386':    {'bits': 32, 'family': 'x86', 'qemu': ['qemu-i386-static', 'qemu-i386']},
    'x86_64':  {'bits': 64, 'family': 'x86', 'qemu': ['qemu-x86_64-static', 'qemu-x86_64']},
    'armv7a':  {'bits': 32, 'family': 'arm', 'qemu': ['qemu-arm-static', 'qemu-arm']},
    'aarch64': {'bits': 64, 'family': 'arm', 'qemu': ['qemu-aarch64-static', 'qemu-aarch64']},
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
    elif machine in ('armv7l', 'armv7a'):
        return os_name, 'arm', 32
    return os_name, machine, 64


def read_elf_entry(path):
    with open(path, 'rb') as f:
        if f.read(4) != b'\x7fELF':
            return None
        ei_class = struct.unpack('B', f.read(1))[0]
        f.seek(0x18)
        if ei_class == 1:
            return struct.unpack('<I', f.read(4))[0]
        else:
            return struct.unpack('<Q', f.read(8))[0]


def read_pe_entry(path):
    with open(path, 'rb') as f:
        if f.read(2) != b'MZ':
            return None
        f.seek(0x3C)
        pe_off = struct.unpack('<I', f.read(4))[0]
        f.seek(pe_off)
        if f.read(4) != b'PE\x00\x00':
            return None
        f.seek(pe_off + 0x28)
        return struct.unpack('<I', f.read(4))[0]


def get_entry_offset(bin_path):
    base = bin_path.rsplit('.', 1)[0]
    for ext, reader in [('.elf', read_elf_entry), ('.exe', read_pe_entry)]:
        path = base + ext
        if os.path.exists(path):
            entry = reader(path)
            if entry:
                return entry, path
    sys.exit(f"[-] Cannot find .elf or .exe to read entry point")


def needs_qemu(host_family, host_bits, target_arch):
    target = ARCH[target_arch]
    if target['family'] != host_family:
        return True
    if target['bits'] != host_bits:
        return True
    return False


def run_qemu(elf_path, qemu_cmds):
    qemu_cmd = None
    for cmd in qemu_cmds:
        if shutil.which(cmd):
            qemu_cmd = cmd
            break

    if not qemu_cmd:
        sys.exit(f"[-] QEMU not found. Install qemu-user-static.")

    print(f"[+] Using: {qemu_cmd}")
    print("[*] Executing...")
    sys.stdout.flush()

    return subprocess.run([qemu_cmd, elf_path]).returncode


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
    parser.add_argument('--arch', required=True, choices=list(ARCH.keys()))
    parser.add_argument('shellcode')
    args = parser.parse_args()

    host_os, host_family, host_bits = get_host()
    target = ARCH[args.arch]

    print(f"[*] Host: {host_os}/{host_family}/{host_bits}bit")
    print(f"[*] Target: {args.arch}")

    entry_offset, exe_path = get_entry_offset(args.shellcode)
    print(f"[+] Entry offset: 0x{entry_offset:x}")

    with open(args.shellcode, 'rb') as f:
        shellcode = f.read()
    print(f"[+] Loaded: {len(shellcode)} bytes")

    if host_os == 'windows':
        code = run_virtualalloc(shellcode, entry_offset)
    elif needs_qemu(host_family, host_bits, args.arch):
        code = run_qemu(exe_path, target['qemu'])
    else:
        code = run_mmap(shellcode, entry_offset)

    print(f"[+] Exit: {code}")
    sys.exit(code)


if __name__ == '__main__':
    main()
