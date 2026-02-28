#pragma once

#include "primitives.h"
#include "span.h"
#include "memory.h"

class BinaryWriter
{
private:
	PVOID address;
	USIZE offset;
	USIZE maxSize;

public:
	constexpr BinaryWriter(PVOID address, USIZE offset, USIZE maxSize)
		: address(address), offset(offset), maxSize(maxSize)
	{
	}

	constexpr BinaryWriter(PVOID address, USIZE maxSize)
		: address(address), offset(0), maxSize(maxSize)
	{
	}

	template <typename T>
	PVOID Write(T value)
	{
		if (offset + sizeof(T) > maxSize)
			return nullptr;

		Memory::Copy((PCHAR)address + offset, &value, sizeof(T));
		offset += sizeof(T);
		return address;
	}

	PVOID WriteBytes(Span<const CHAR> data)
	{
		if (offset + data.Size() > maxSize)
			return nullptr;

		Memory::Copy((PCHAR)address + offset, data.Data(), data.Size());
		offset += data.Size();
		return address;
	}

	FORCE_INLINE PVOID WriteU8(UINT8 value)
	{
		if (offset + 1 > maxSize)
			return nullptr;

		*((PUCHAR)address + offset) = value;
		offset += 1;
		return address;
	}

	FORCE_INLINE PVOID WriteU16BE(UINT16 value)
	{
		if (offset + 2 > maxSize)
			return nullptr;

		PUCHAR p = (PUCHAR)address + offset;
		p[0] = (UINT8)(value >> 8);
		p[1] = (UINT8)(value & 0xFF);
		offset += 2;
		return address;
	}

	FORCE_INLINE PVOID WriteU24BE(UINT32 value)
	{
		if (offset + 3 > maxSize)
			return nullptr;

		PUCHAR p = (PUCHAR)address + offset;
		p[0] = (UINT8)((value >> 16) & 0xFF);
		p[1] = (UINT8)((value >> 8) & 0xFF);
		p[2] = (UINT8)(value & 0xFF);
		offset += 3;
		return address;
	}

	FORCE_INLINE PVOID WriteU32BE(UINT32 value)
	{
		if (offset + 4 > maxSize)
			return nullptr;

		PUCHAR p = (PUCHAR)address + offset;
		p[0] = (UINT8)((value >> 24) & 0xFF);
		p[1] = (UINT8)((value >> 16) & 0xFF);
		p[2] = (UINT8)((value >> 8) & 0xFF);
		p[3] = (UINT8)(value & 0xFF);
		offset += 4;
		return address;
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

	constexpr PVOID GetAddress() const { return address; }
	constexpr USIZE GetOffset() const { return offset; }
	constexpr USIZE GetMaxSize() const { return maxSize; }
};
