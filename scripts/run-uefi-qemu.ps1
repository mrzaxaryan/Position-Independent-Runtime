#
# run-uefi-qemu.ps1 - Launch UEFI build in QEMU with OVMF firmware (Windows)
#
# Usage:
#   .\run-uefi-qemu.ps1 [architecture] [build_type]
#
# Arguments:
#   architecture  - Target architecture: x86_64, i386, aarch64 (default: x86_64)
#   build_type    - Build type: release, debug (default: release)
#
# Examples:
#   .\run-uefi-qemu.ps1                    # x86_64 release
#   .\run-uefi-qemu.ps1 aarch64            # aarch64 release
#   .\run-uefi-qemu.ps1 x86_64 debug       # x86_64 debug

param(
    [string]$Architecture = "x86_64",
    [string]$BuildType = "release"
)

$ErrorActionPreference = "Stop"

# Normalize build type
$BuildTypeLower = $BuildType.ToLower()

# Validate architecture
$ValidArchs = @("x86_64", "i386", "aarch64")
if ($Architecture -notin $ValidArchs) {
    Write-Host "Error: Invalid architecture '$Architecture'" -ForegroundColor Red
    Write-Host "Valid options: x86_64, i386, aarch64"
    exit 1
}

# Build directory structure
$BuildDir = "build\uefi\$Architecture\$BuildTypeLower"
$OutputEfi = "$BuildDir\output.efi"

# Check if build exists
if (-not (Test-Path $OutputEfi)) {
    Write-Host "Error: UEFI binary not found at: $OutputEfi" -ForegroundColor Red
    Write-Host ""
    Write-Host "Build it first with:"
    Write-Host "  cmake -B $BuildDir\cmake -G Ninja ``"
    Write-Host "    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-clang.cmake ``"
    Write-Host "    -DARCHITECTURE=$Architecture ``"
    Write-Host "    -DPLATFORM=uefi ``"
    Write-Host "    -DBUILD_TYPE=$BuildTypeLower"
    Write-Host "  cmake --build $BuildDir\cmake"
    exit 1
}

Write-Host "=== CPP-PIC UEFI Test Runner ===" -ForegroundColor Cyan
Write-Host "Architecture:  $Architecture"
Write-Host "Build Type:    $BuildTypeLower"
Write-Host "UEFI Binary:   $OutputEfi"
Write-Host ""

# Create temporary directory for UEFI disk image
$TempDir = New-Item -ItemType Directory -Path ([System.IO.Path]::Combine($env:TEMP, [System.IO.Path]::GetRandomFileName()))
try {
    $UefiImg = Join-Path $TempDir "uefi.img"
    $MountDir = Join-Path $TempDir "mnt"

    # Create FAT filesystem image (64MB) using fsutil on Windows
    Write-Host "[1/4] Creating FAT filesystem image..." -ForegroundColor Yellow

    # Create 64MB file
    $ImgSize = 64MB
    $null = New-Item -ItemType File -Path $UefiImg -Force
    fsutil file createnew $UefiImg $ImgSize | Out-Null

    # Format as FAT32 using diskpart (requires admin) or use pre-formatted approach
    # For simplicity, we'll create the directory structure manually in the image
    # Note: On Windows, we'll need to use a different approach or require admin rights

    # Alternative: Use mtools (if available) or create ISO
    # For this script, we'll use a simpler approach: create directory structure

    New-Item -ItemType Directory -Path $MountDir -Force | Out-Null
    $EfiBootDir = Join-Path $MountDir "EFI\BOOT"
    New-Item -ItemType Directory -Path $EfiBootDir -Force | Out-Null

    # Determine boot filename
    $BootFile = switch ($Architecture) {
        "x86_64"  { "BOOTX64.EFI" }
        "i386"    { "BOOTIA32.EFI" }
        "aarch64" { "BOOTAA64.EFI" }
    }

    Write-Host "[2/4] Installing UEFI application..." -ForegroundColor Yellow
    Copy-Item $OutputEfi -Destination (Join-Path $EfiBootDir $BootFile)
    Write-Host "   Installed as: \EFI\BOOT\$BootFile"

    # Create an ESP (EFI System Partition) image using alternative method
    # We'll create a simple FAT image that QEMU can boot from

    # For Windows, we need to use QEMU's built-in support or create an ISO
    # Let's create using a different approach - direct directory boot if QEMU supports it

    Write-Host "[3/4] Detecting QEMU and OVMF firmware..." -ForegroundColor Yellow

    # Detect QEMU installation
    $QemuBin = $null
    $OvmfCode = $null

    # Common QEMU installation paths on Windows
    $QemuPaths = @(
        "C:\Program Files\qemu",
        "C:\qemu",
        "$env:ProgramFiles\qemu",
        "${env:ProgramFiles(x86)}\qemu"
    )

    foreach ($path in $QemuPaths) {
        $testBin = switch ($Architecture) {
            "x86_64"  { Join-Path $path "qemu-system-x86_64.exe" }
            "i386"    { Join-Path $path "qemu-system-i386.exe" }
            "aarch64" { Join-Path $path "qemu-system-aarch64.exe" }
        }

        if (Test-Path $testBin) {
            $QemuBin = $testBin

            # Look for OVMF in QEMU directory
            $ovmfPaths = switch ($Architecture) {
                "x86_64" {
                    @(
                        (Join-Path $path "share\edk2-x86_64-code.fd"),
                        (Join-Path $path "share\OVMF_CODE.fd"),
                        (Join-Path $path "edk2-x86_64-code.fd")
                    )
                }
                "i386" {
                    @(
                        (Join-Path $path "share\edk2-i386-code.fd"),
                        (Join-Path $path "edk2-i386-code.fd")
                    )
                }
                "aarch64" {
                    @(
                        (Join-Path $path "share\edk2-aarch64-code.fd"),
                        (Join-Path $path "edk2-aarch64-code.fd")
                    )
                }
            }

            foreach ($ovmf in $ovmfPaths) {
                if (Test-Path $ovmf) {
                    $OvmfCode = $ovmf
                    break
                }
            }

            if ($OvmfCode) {
                break
            }
        }
    }

    # Check in PATH
    if (-not $QemuBin) {
        $QemuBin = switch ($Architecture) {
            "x86_64"  { (Get-Command "qemu-system-x86_64.exe" -ErrorAction SilentlyContinue).Source }
            "i386"    { (Get-Command "qemu-system-i386.exe" -ErrorAction SilentlyContinue).Source }
            "aarch64" { (Get-Command "qemu-system-aarch64.exe" -ErrorAction SilentlyContinue).Source }
        }
    }

    if (-not $QemuBin) {
        Write-Host "Error: QEMU not found for ${Architecture}" -ForegroundColor Red
        Write-Host ""
        Write-Host "Install QEMU:"
        Write-Host "  choco install qemu"
        Write-Host "  Or download from: https://qemu.weilnetz.de/"
        exit 1
    }

    if (-not $OvmfCode) {
        Write-Host "Warning: OVMF firmware not found, using legacy BIOS boot" -ForegroundColor Yellow
    }

    Write-Host "   QEMU:     $QemuBin"
    if ($OvmfCode) {
        Write-Host "   Firmware: $OvmfCode"
    }
    Write-Host ""

    Write-Host "[4/4] Launching QEMU..." -ForegroundColor Yellow
    Write-Host "=== Starting QEMU (press Ctrl+C to exit) ===" -ForegroundColor Cyan
    Write-Host ""

    # Build QEMU command
    $QemuArgs = @()

    switch ($Architecture) {
        "x86_64" {
            $QemuArgs += @("-machine", "q35", "-m", "512M")
        }
        "i386" {
            $QemuArgs += @("-machine", "q35", "-m", "512M")
        }
        "aarch64" {
            $QemuArgs += @("-machine", "virt", "-cpu", "cortex-a57", "-m", "512M")
        }
    }

    # Add firmware
    if ($OvmfCode) {
        $QemuArgs += @("-drive", "if=pflash,format=raw,readonly=on,file=$OvmfCode")
    }

    # For Windows, we use direct directory boot or create a simple disk
    # QEMU on Windows can boot from a directory with -hda pointing to ESP structure
    $QemuArgs += @(
        "-drive", "file=fat:rw:$MountDir,format=raw",
        "-nographic",
        "-serial", "stdio",
        "-monitor", "none"
    )

    # Execute QEMU
    & $QemuBin @QemuArgs

    Write-Host ""
    Write-Host "=== QEMU session ended ===" -ForegroundColor Cyan

} finally {
    # Cleanup
    if (Test-Path $TempDir) {
        Remove-Item -Recurse -Force $TempDir
    }
}
