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
	BinaryReader(PVOID address, USIZE offset, USIZE maxSize)
		: address(address), offset(offset), maxSize(maxSize)
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

	PVOID GetAddress() const { return address; }
	USIZE GetOffset() const { return offset; }
	USIZE GetMaxSize() const { return maxSize; }
};
