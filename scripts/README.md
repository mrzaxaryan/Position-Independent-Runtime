# Scripts Directory

Automation scripts for building, testing, and deploying CPP-PIC.

## Scripts

### Setup Scripts

#### [install.sh](install.sh)
Installs LLVM 20 toolchain on Linux systems.

```bash
# Run on Linux/WSL
./scripts/install.sh
```

**What it does:**
- Adds LLVM APT repository
- Installs Clang 20, LLD 20, LLVM 20
- Sets up development environment

---

### Testing Scripts

#### [run-uefi-qemu.ps1](run-uefi-qemu.ps1)
Builds and tests UEFI application in QEMU (Windows).

```powershell
# Test x86_64 UEFI in debug mode
.\scripts\run-uefi-qemu.ps1 -Architecture x86_64

# Test aarch64 UEFI in release mode
.\scripts\run-uefi-qemu.ps1 -Architecture aarch64 -BuildType release
```

**Parameters:**
- `-Architecture` - Target architecture (x86_64, aarch64, i386)
- `-BuildType` - Build type (debug, release) - Default: debug

**What it does:**
1. Compiles UEFI application
2. Creates FAT32 disk image
3. Copies EFI file to `EFI\BOOT\BOOT{ARCH}.EFI`
4. Launches QEMU with OVMF firmware
5. Captures console output

#### [run-uefi-qemu.sh](run-uefi-qemu.sh)
Builds and tests UEFI application in QEMU (Linux/macOS).

```bash
# Test x86_64 UEFI
./scripts/run-uefi-qemu.sh x86_64

# Test aarch64 UEFI
./scripts/run-uefi-qemu.sh aarch64
```

**Parameters:**
- `$1` - Architecture (x86_64, aarch64, i386)

---

### Deployment Scripts

#### [loader.ps1](loader.ps1)
Loads and executes PIC blob in memory (Windows).

```powershell
# Load base64-encoded PIC blob
.\scripts\loader.ps1 -BlobPath .\build\windows\x86_64\debug\output.b64.txt

# Load raw binary blob
.\scripts\loader.ps1 -BlobPath .\build\windows\x86_64\debug\output.bin -IsRaw
```

**Parameters:**
- `-BlobPath` - Path to PIC blob file (base64 or raw)
- `-IsRaw` - Switch to indicate raw binary (default: base64)

**What it does:**
1. Reads PIC blob file
2. Decodes base64 (if applicable)
3. Allocates executable memory (VirtualAlloc)
4. Copies blob to memory
5. Creates delegate and executes
6. Reports exit code

---

## Usage Examples

### Complete Build & Test Workflow

```powershell
# 1. Build all Windows configurations
.\compile.bat x86_64 windows DEBUG
.\compile.bat x86_64 windows RELEASE

# 2. Test UEFI
.\scripts\run-uefi-qemu.ps1 -Architecture x86_64

# 3. Load PIC blob
.\scripts\loader.ps1 -BlobPath .\build\windows\x86_64\debug\output.b64.txt
```

### Linux Development

```bash
# 1. Install toolchain (first time only)
./scripts/install.sh

# 2. Build Linux binary
cmake -B build/linux/x86_64/debug \
      -DARCHITECTURE=x86_64 \
      -DPLATFORM=linux \
      -DBUILD_TYPE=debug

cmake --build build/linux/x86_64/debug

# 3. Run tests
./build/linux/x86_64/debug/output.elf

# 4. Test UEFI
./scripts/run-uefi-qemu.sh x86_64
```

---

## Script Dependencies

### Windows Scripts (PowerShell)

**Required:**
- PowerShell 5.1+ (Windows 10+)
- LLVM/Clang 20+ (in PATH)
- CMake 3.20+

**Optional:**
- QEMU (for UEFI testing) - `run-uefi-qemu.ps1`
  - Download: https://qemu.weilnetz.de/w64/
  - Add to PATH: `C:\Program Files\qemu`

### Linux Scripts (Bash)

**Required:**
- Bash 4.0+
- LLVM/Clang 20+ (via `install.sh`)
- CMake 3.20+

**Optional:**
- QEMU (for UEFI testing) - `run-uefi-qemu.sh`
  ```bash
  sudo apt install qemu-system-x86 qemu-system-arm ovmf
  ```

---

## Adding New Scripts

### Naming Conventions

- **Linux/macOS**: `.sh` extension, executable (`chmod +x`)
- **Windows**: `.ps1` extension (PowerShell)
- **Cross-platform**: Create both `.sh` and `.ps1` versions

### Script Template

**PowerShell** (`.ps1`):
```powershell
#!/usr/bin/env pwsh
# Script description

param(
    [Parameter(Mandatory=$true)]
    [string]$RequiredParam,

    [string]$OptionalParam = "default"
)

# Script logic
Write-Host "Running script..."
```

**Bash** (`.sh`):
```bash
#!/bin/bash
# Script description

set -e  # Exit on error

REQUIRED_PARAM=$1
OPTIONAL_PARAM=${2:-"default"}

# Script logic
echo "Running script..."
```

### Best Practices

1. **Add error handling** - Check for required tools
2. **Provide usage info** - Show help on `-h` or `--help`
3. **Use absolute paths** - Avoid issues with working directory
4. **Document parameters** - Clear descriptions in comments
5. **Test on target platforms** - Verify scripts work as expected

---

## Troubleshooting

### QEMU Scripts

**Problem**: "QEMU not found"
- **Solution**: Install QEMU and add to PATH
- **Windows**: Download from https://qemu.weilnetz.de/w64/
- **Linux**: `sudo apt install qemu-system-x86 qemu-system-arm`

**Problem**: "OVMF not found"
- **Solution**: OVMF firmware is required for UEFI
- **Windows**: Download OVMF.fd from EDK2 releases
- **Linux**: `sudo apt install ovmf`

### Loader Script

**Problem**: "Access denied"
- **Solution**: Run PowerShell as Administrator
- **Reason**: VirtualAlloc with PAGE_EXECUTE_READWRITE requires privileges

**Problem**: "Invalid blob format"
- **Solution**: Verify blob file is valid base64 or raw binary
- **Debug**: Check `output.b64.txt` is generated correctly

### Build Scripts

**Problem**: "Clang not found"
- **Solution**: Install LLVM 20 and add to PATH
- **Windows**: Download from https://github.com/llvm/llvm-project/releases
- **Linux**: Run `./scripts/install.sh`

---

## References

- [Build Guide](../README.md) - Main build instructions
- [Architecture Guide](../docs/architecture.md) - System architecture
- [Platform Guide](../docs/platform_guide.md) - Platform-specific details
