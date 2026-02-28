#pragma once

#include "primitives.h"
#include "span.h"
#include "memory.h"

class BinaryReader
{
private:
	PVOID address;
	USIZE offset;
	USIZE maxSize;

public:
	constexpr BinaryReader(PVOID address, USIZE offset, USIZE maxSize)
		: address(address), offset(offset), maxSize(maxSize)
	{
	}

	constexpr BinaryReader(PVOID address, USIZE maxSize)
		: address(address), offset(0), maxSize(maxSize)
	{
	}

	template <typename T>
	T Read()
	{
		if (offset + sizeof(T) > maxSize)
			return T{};

		T value;
		Memory::Copy(&value, (PCHAR)address + offset, sizeof(T));
		offset += sizeof(T);
		return value;
	}

	USIZE ReadBytes(Span<CHAR> buffer)
	{
		if (offset + buffer.Size() > maxSize)
			return 0;

		Memory::Copy(buffer.Data(), (PCHAR)address + offset, buffer.Size());
		offset += buffer.Size();
		return buffer.Size();
	}

	FORCE_INLINE UINT16 ReadU16BE()
	{
		if (offset + 2 > maxSize)
			return 0;

		PUCHAR p = (PUCHAR)address + offset;
		UINT16 value = (UINT16)((p[0] << 8) | p[1]);
		offset += 2;
		return value;
	}

	FORCE_INLINE UINT32 ReadU24BE()
	{
		if (offset + 3 > maxSize)
			return 0;

		PUCHAR p = (PUCHAR)address + offset;
		UINT32 value = ((UINT32)p[0] << 16) | ((UINT32)p[1] << 8) | (UINT32)p[2];
		offset += 3;
		return value;
	}

	FORCE_INLINE UINT32 ReadU32BE()
	{
		if (offset + 4 > maxSize)
			return 0;

		PUCHAR p = (PUCHAR)address + offset;
		UINT32 value = ((UINT32)p[0] << 24) | ((UINT32)p[1] << 16) | ((UINT32)p[2] << 8) | (UINT32)p[3];
		offset += 4;
		return value;
	}

	FORCE_INLINE BOOL Skip(USIZE count)
	{
		if (offset + count > maxSize)
			return false;

		offset += count;
		return true;
	}

	constexpr USIZE Remaining() const
	{
		return (offset < maxSize) ? (maxSize - offset) : 0;
	}

	FORCE_INLINE BOOL SetOffset(USIZE newOffset)
	{
		if (newOffset > maxSize)
			return false;

		offset = newOffset;
		return true;
	}

	constexpr PVOID Current() const
	{
		return (PCHAR)address + offset;
	}

	constexpr PVOID GetAddress() const { return address; }
	constexpr USIZE GetOffset() const { return offset; }
	constexpr USIZE GetMaxSize() const { return maxSize; }
};
