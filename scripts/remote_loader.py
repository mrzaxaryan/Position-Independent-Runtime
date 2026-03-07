#!/usr/bin/env python3
"""
Remote PIC Shellcode Loader

Downloads the correct .bin from GitHub Releases and executes it.
No authentication required.

Usage:
    python remote_loader.py
    python remote_loader.py --arch x86_64
    python remote_loader.py --tag v0.0.1-alpha.1
"""

import argparse
import os
import platform
import struct
import sys
import urllib.error
import urllib.request

# Import execution functions from loader.py
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from loader import ARCH, get_host, run_mmap, run_injected

REPO = "mrzaxaryan/Position-Independent-Runtime"

# Map (os_name, family, bits) -> (platform_name, arch_name)
_HOST_TO_ARTIFACT = {
    ('linux',   'x86',   64): ('linux',   'x86_64'),
    ('linux',   'x86',   32): ('linux',   'i386'),
    ('linux',   'arm',   64): ('linux',   'aarch64'),
    ('linux',   'arm',   32): ('linux',   'armv7a'),
    ('linux',   'riscv', 64): ('linux',   'riscv64'),
    ('linux',   'riscv', 32): ('linux',   'riscv32'),
    ('windows', 'x86',   64): ('windows', 'x86_64'),
    ('windows', 'x86',   32): ('windows', 'i386'),
    ('windows', 'arm',   64): ('windows', 'aarch64'),
    ('windows', 'arm',   32): ('windows', 'armv7a'),
    ('darwin',  'x86',   64): ('macos',   'x86_64'),
    ('darwin',  'arm',   64): ('macos',   'aarch64'),
    ('freebsd', 'x86',   64): ('freebsd', 'x86_64'),
    ('freebsd', 'x86',   32): ('freebsd', 'i386'),
    ('freebsd', 'arm',   64): ('freebsd', 'aarch64'),
    ('freebsd', 'riscv', 64): ('freebsd', 'riscv64'),
    ('sunos',   'x86',   64): ('solaris', 'x86_64'),
    ('sunos',   'x86',   32): ('solaris', 'i386'),
    ('sunos',   'arm',   64): ('solaris', 'aarch64'),
}


def download_release_asset(platform_name, arch, tag):
    """Download .bin from a GitHub Release (no auth needed)."""
    asset = f"{platform_name}-{arch}.bin"
    if tag:
        url = f"https://github.com/{REPO}/releases/download/{tag}/{asset}"
    else:
        url = f"https://github.com/{REPO}/releases/latest/download/{asset}"

    print(f"[*] Downloading: {asset}")
    print(f"[*] URL: {url}")
    req = urllib.request.Request(url, headers={"User-Agent": "PIR-RemoteLoader/1.0"})
    try:
        with urllib.request.urlopen(req) as resp:
            return resp.read()
    except urllib.error.HTTPError as e:
        if e.code == 404:
            sys.exit(
                f"[-] Asset '{asset}' not found.\n"
                f"    Make sure a release exists with this asset.\n"
                f"    URL: {url}"
            )
        raise


def main():
    parser = argparse.ArgumentParser(description='Remote PIC Shellcode Loader')
    parser.add_argument('--arch', choices=list(ARCH.keys()),
                        help='Target architecture (auto-detected if omitted)')
    parser.add_argument('--tag', default=None,
                        help='Release tag (default: latest)')
    args = parser.parse_args()

    host_os, host_family, host_bits = get_host()
    python_bits = struct.calcsize("P") * 8

    print(f"[*] Host: {host_os}/{host_family}/{host_bits}bit")
    print(f"[*] Python: {python_bits}bit")

    # Determine target platform and arch
    host_key = (host_os, host_family, host_bits)
    if host_key not in _HOST_TO_ARTIFACT:
        sys.exit(f"[-] Unsupported host: {host_os}/{host_family}/{host_bits}bit")

    platform_name, detected_arch = _HOST_TO_ARTIFACT[host_key]
    target_arch = args.arch or detected_arch

    print(f"[*] Platform: {platform_name}")
    print(f"[*] Target arch: {target_arch}")
    print(f"[*] Release: {args.tag or 'latest'}")

    # Download .bin from release
    shellcode = download_release_asset(platform_name, target_arch, args.tag)
    print(f"[+] Loaded: {len(shellcode)} bytes")

    # Execute
    if host_os == 'windows':
        target = ARCH[target_arch]
        cross_family = host_family != target['family']
        code = run_injected(shellcode, target_arch, cross_family=cross_family)
    elif target_arch != detected_arch:
        sys.exit(f"[-] Cannot load {target_arch} shellcode on {host_family}/{host_bits}bit host")
    else:
        code = run_mmap(shellcode)

    print(f"[+] Exit: {code}")
    os._exit(code)


if __name__ == '__main__':
    main()
