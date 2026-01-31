#!/usr/bin/env python3
"""
UEFI QEMU Runner

Runs UEFI applications in QEMU for testing in WSL.

Usage:
    python run_uefi.py --arch x86_64 --build release
    python run_uefi.py --arch aarch64 --build debug
"""

import argparse
import os
import subprocess
import sys

QEMU_CONFIG = {
    'x86_64': {
        'qemu': 'qemu-system-x86_64',
        'bios': '/usr/share/ovmf/OVMF.fd',
        'extra_args': [],
    },
    'aarch64': {
        'qemu': 'qemu-system-aarch64',
        'bios': '/usr/share/qemu-efi-aarch64/QEMU_EFI.fd',
        'extra_args': ['-M', 'virt', '-cpu', 'cortex-a72'],
    },
}


def main():
    parser = argparse.ArgumentParser(description='UEFI QEMU Runner')
    parser.add_argument('--arch', required=True, choices=list(QEMU_CONFIG.keys()),
                        help='Target architecture')
    parser.add_argument('--build', required=True, choices=['debug', 'release'],
                        help='Build type')
    parser.add_argument('--workdir', default=os.getcwd(),
                        help='Working directory (project root)')
    args = parser.parse_args()

    config = QEMU_CONFIG[args.arch]
    build_dir = os.path.join(args.workdir, 'build', args.build, 'uefi', args.arch)

    if not os.path.isdir(build_dir):
        sys.exit(f"[-] Build directory not found: {build_dir}\n    Run the build task first.")

    if not os.path.isfile(config['bios']):
        sys.exit(f"[-] UEFI firmware not found: {config['bios']}\n"
                 f"    Install with: sudo apt install ovmf qemu-efi-aarch64")

    # Build QEMU command
    cmd = [
        config['qemu'],
        *config['extra_args'],
        '-bios', config['bios'],
        '-drive', f"format=raw,file=fat:rw:{build_dir}",
        '-nographic',
        '-device', 'virtio-net-pci,netdev=net0,mac=52:54:00:12:34:56' if args.arch == 'aarch64' else 'e1000,netdev=net0,mac=52:54:00:12:34:56',
        '-netdev', 'user,id=net0,net=10.0.2.0/24,dhcpstart=10.0.2.15,dns=10.0.2.3',
    ]

    print(f"[*] Architecture: {args.arch}")
    print(f"[*] Build: {args.build}")
    print(f"[*] Build dir: {build_dir}")
    print(f"[*] Running: {' '.join(cmd)}")
    print("[*] Press Ctrl+A then X to exit QEMU")
    print("-" * 60)
    sys.stdout.flush()

    return subprocess.run(cmd).returncode


if __name__ == '__main__':
    sys.exit(main())
