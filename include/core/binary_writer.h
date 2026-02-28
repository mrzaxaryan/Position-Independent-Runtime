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
	BinaryWriter(PVOID address, USIZE offset, USIZE maxSize)
		: address(address), offset(offset), maxSize(maxSize)
	{
	}

	template <typename T>
	PVOID Write(T value)
	{
		if (offset + sizeof(T) > maxSize)
			return nullptr;

		*(T *)((PCHAR)address + offset) = value;
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

	PVOID GetAddress() const { return address; }
	USIZE GetOffset() const { return offset; }
	USIZE GetMaxSize() const { return maxSize; }
};
