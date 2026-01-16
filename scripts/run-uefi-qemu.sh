#!/usr/bin/env bash
#
# run-uefi-qemu.sh - Launch UEFI build in QEMU with OVMF firmware
#
# Usage:
#   ./run-uefi-qemu.sh [architecture] [build_type]
#
# Arguments:
#   architecture  - Target architecture: x86_64, i386, aarch64 (default: x86_64)
#   build_type    - Build type: release, debug (default: release)
#
# Examples:
#   ./run-uefi-qemu.sh                    # x86_64 release
#   ./run-uefi-qemu.sh aarch64            # aarch64 release
#   ./run-uefi-qemu.sh x86_64 debug       # x86_64 debug

set -euo pipefail

# Configuration
ARCH="${1:-x86_64}"
BUILD_TYPE="${2:-release}"
BUILD_TYPE_LOWER="$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"

# Validate architecture
case "$ARCH" in
    x86_64|i386|aarch64)
        ;;
    *)
        echo "Error: Invalid architecture '$ARCH'"
        echo "Valid options: x86_64, i386, aarch64"
        exit 1
        ;;
esac

# Build directory structure
BUILD_DIR="build/uefi/${ARCH}/${BUILD_TYPE_LOWER}"
OUTPUT_EFI="${BUILD_DIR}/output.efi"

# Check if build exists
if [[ ! -f "$OUTPUT_EFI" ]]; then
    echo "Error: UEFI binary not found at: $OUTPUT_EFI"
    echo ""
    echo "Build it first with:"
    echo "  cmake -B ${BUILD_DIR}/cmake -G Ninja \\"
    echo "    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-clang.cmake \\"
    echo "    -DARCHITECTURE=${ARCH} \\"
    echo "    -DPLATFORM=uefi \\"
    echo "    -DBUILD_TYPE=${BUILD_TYPE_LOWER}"
    echo "  cmake --build ${BUILD_DIR}/cmake"
    exit 1
fi

# Create temporary directory for UEFI disk image
TEMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TEMP_DIR"' EXIT

UEFI_IMG="${TEMP_DIR}/uefi.img"
MOUNT_DIR="${TEMP_DIR}/mnt"

echo "=== CPP-PIC UEFI Test Runner ==="
echo "Architecture:  ${ARCH}"
echo "Build Type:    ${BUILD_TYPE_LOWER}"
echo "UEFI Binary:   ${OUTPUT_EFI}"
echo ""

# Create FAT filesystem image (64MB)
echo "[1/5] Creating FAT filesystem image..."
dd if=/dev/zero of="$UEFI_IMG" bs=1M count=64 status=none
mkfs.vfat "$UEFI_IMG" >/dev/null 2>&1

# Mount the image
echo "[2/5] Mounting filesystem..."
mkdir -p "$MOUNT_DIR"
sudo mount -o loop "$UEFI_IMG" "$MOUNT_DIR"
trap 'sudo umount "$MOUNT_DIR" 2>/dev/null || true; rm -rf "$TEMP_DIR"' EXIT

# Copy UEFI binary to correct location
echo "[3/5] Installing UEFI application..."
sudo mkdir -p "$MOUNT_DIR/EFI/BOOT"

# Copy to architecture-specific boot file
case "$ARCH" in
    x86_64)
        BOOT_FILE="BOOTX64.EFI"
        ;;
    i386)
        BOOT_FILE="BOOTIA32.EFI"
        ;;
    aarch64)
        BOOT_FILE="BOOTAA64.EFI"
        ;;
esac

sudo cp "$OUTPUT_EFI" "$MOUNT_DIR/EFI/BOOT/$BOOT_FILE"
echo "   Installed as: /EFI/BOOT/$BOOT_FILE"

# Unmount
echo "[4/5] Unmounting filesystem..."
sudo umount "$MOUNT_DIR"
trap 'rm -rf "$TEMP_DIR"' EXIT

# Detect OVMF firmware location
echo "[5/5] Launching QEMU with OVMF firmware..."
OVMF_CODE=""
OVMF_VARS=""

case "$ARCH" in
    x86_64)
        # Common OVMF locations for x86_64
        for path in \
            "/usr/share/OVMF/OVMF_CODE.fd" \
            "/usr/share/ovmf/OVMF.fd" \
            "/usr/share/ovmf/x64/OVMF_CODE.fd" \
            "/usr/share/edk2-ovmf/x64/OVMF_CODE.fd" \
            "/usr/share/qemu/ovmf-x86_64-code.bin"
        do
            if [[ -f "$path" ]]; then
                OVMF_CODE="$path"
                break
            fi
        done

        # Variables file (optional, use template if available)
        for path in \
            "/usr/share/OVMF/OVMF_VARS.fd" \
            "/usr/share/ovmf/x64/OVMF_VARS.fd" \
            "/usr/share/edk2-ovmf/x64/OVMF_VARS.fd"
        do
            if [[ -f "$path" ]]; then
                # Create temporary copy of variables
                OVMF_VARS="${TEMP_DIR}/OVMF_VARS.fd"
                cp "$path" "$OVMF_VARS"
                break
            fi
        done

        QEMU_BIN="qemu-system-x86_64"
        QEMU_ARGS="-machine q35 -m 512M"
        ;;

    i386)
        # OVMF for i386
        for path in \
            "/usr/share/OVMF/OVMF32_CODE.fd" \
            "/usr/share/ovmf/ia32/OVMF_CODE.fd" \
            "/usr/share/edk2-ovmf/ia32/OVMF_CODE.fd"
        do
            if [[ -f "$path" ]]; then
                OVMF_CODE="$path"
                break
            fi
        done

        QEMU_BIN="qemu-system-i386"
        QEMU_ARGS="-machine q35 -m 512M"
        ;;

    aarch64)
        # AAVMF (ARM64 UEFI firmware)
        for path in \
            "/usr/share/AAVMF/AAVMF_CODE.fd" \
            "/usr/share/qemu-efi-aarch64/QEMU_EFI.fd" \
            "/usr/share/edk2-aarch64/QEMU_EFI.fd"
        do
            if [[ -f "$path" ]]; then
                OVMF_CODE="$path"
                break
            fi
        done

        # Variables file for ARM64
        for path in \
            "/usr/share/AAVMF/AAVMF_VARS.fd" \
            "/usr/share/qemu-efi-aarch64/vars-template-pflash.raw"
        do
            if [[ -f "$path" ]]; then
                OVMF_VARS="${TEMP_DIR}/AAVMF_VARS.fd"
                cp "$path" "$OVMF_VARS"
                break
            fi
        done

        QEMU_BIN="qemu-system-aarch64"
        QEMU_ARGS="-machine virt -cpu cortex-a57 -m 512M"
        ;;
esac

# Verify OVMF firmware exists
if [[ -z "$OVMF_CODE" ]]; then
    echo "Error: OVMF firmware not found for ${ARCH}"
    echo ""
    echo "Install OVMF firmware:"
    case "$ARCH" in
        x86_64)
            echo "  Ubuntu/Debian: sudo apt install ovmf"
            echo "  Fedora:        sudo dnf install edk2-ovmf"
            echo "  Arch:          sudo pacman -S edk2-ovmf"
            ;;
        i386)
            echo "  Ubuntu/Debian: sudo apt install ovmf-ia32"
            ;;
        aarch64)
            echo "  Ubuntu/Debian: sudo apt install qemu-efi-aarch64"
            echo "  Fedora:        sudo dnf install edk2-aarch64"
            ;;
    esac
    exit 1
fi

echo "   QEMU:     $QEMU_BIN"
echo "   Firmware: $OVMF_CODE"
if [[ -n "$OVMF_VARS" ]]; then
    echo "   Variables: $OVMF_VARS"
fi
echo ""
echo "=== Starting QEMU (press Ctrl+A then X to exit) ==="
echo ""

# Build QEMU command
QEMU_CMD=(
    "$QEMU_BIN"
    $QEMU_ARGS
    -nographic
    -serial mon:stdio
)

# Add firmware
if [[ -n "$OVMF_VARS" ]]; then
    # Use separate code and vars
    QEMU_CMD+=(
        -drive "if=pflash,format=raw,readonly=on,file=$OVMF_CODE"
        -drive "if=pflash,format=raw,file=$OVMF_VARS"
    )
else
    # Use single firmware file (legacy)
    QEMU_CMD+=(-bios "$OVMF_CODE")
fi

# Add disk image
QEMU_CMD+=(-drive "format=raw,file=$UEFI_IMG")

# Execute QEMU
"${QEMU_CMD[@]}"

echo ""
echo "=== QEMU session ended ==="
