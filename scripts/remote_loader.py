#!/usr/bin/env python3
"""
Remote PIC Shellcode Loader

Downloads the correct .bin for the current platform/architecture
from GitHub Releases and executes it in memory.
No authentication required.

Usage:
    python remote_loader.py
    python remote_loader.py --tag v0.0.1-alpha.1
"""

import argparse
import json
import os
import platform
import struct
import sys
import urllib.error
import urllib.request

# Import execution functions from loader.py
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from loader import get_host, run_mmap, run_injected

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


def http_get(url):
    """Download URL content."""
    req = urllib.request.Request(url, headers={"User-Agent": "PIR-RemoteLoader/1.0"})
    with urllib.request.urlopen(req) as resp:
        return resp.read()


def find_latest_tag():
    """Find the most recent release tag (including pre-releases)."""
    url = f"https://api.github.com/repos/{REPO}/releases?per_page=1"
    req = urllib.request.Request(url, headers={
        "User-Agent": "PIR-RemoteLoader/1.0",
        "Accept": "application/vnd.github+json",
    })
    with urllib.request.urlopen(req) as resp:
        releases = json.loads(resp.read())
    if not releases:
        sys.exit("[-] No releases found")
    return releases[0]["tag_name"]


def download_release_asset(platform_name, arch, tag):
    """Download .bin from a GitHub Release (no auth needed)."""
    if not tag:
        tag = find_latest_tag()
        print(f"[+] Latest release: {tag}")

    asset = f"{platform_name}-{arch}.bin"
    url = f"https://github.com/{REPO}/releases/download/{tag}/{asset}"

    print(f"[*] Downloading: {asset}")
    print(f"[*] URL: {url}")
    try:
        return http_get(url)
    except urllib.error.HTTPError as e:
        if e.code == 404:
            sys.exit(
                f"[-] Asset '{asset}' not found in release {tag}.\n"
                f"    URL: {url}"
            )
        raise


def main():
    parser = argparse.ArgumentParser(description='Remote PIC Shellcode Loader')
    parser.add_argument('--tag', default=None,
                        help='Release tag (default: latest)')
    args = parser.parse_args()

    host_os, host_family, host_bits = get_host()

    print(f"[*] Host: {host_os}/{host_family}/{host_bits}bit")

    host_key = (host_os, host_family, host_bits)
    if host_key not in _HOST_TO_ARTIFACT:
        sys.exit(f"[-] Unsupported host: {host_os}/{host_family}/{host_bits}bit")

    platform_name, arch = _HOST_TO_ARTIFACT[host_key]

    print(f"[*] Platform: {platform_name}/{arch}")
    print(f"[*] Release: {args.tag or 'latest'}")

    # Download .bin from release into memory
    shellcode = download_release_asset(platform_name, arch, args.tag)
    print(f"[+] Loaded: {len(shellcode)} bytes")

    # Execute
    if host_os == 'windows':
        code = run_injected(shellcode, arch)
    else:
        code = run_mmap(shellcode)

    print(f"[+] Exit: {code}")
    os._exit(code)


if __name__ == '__main__':
    main()
