#include "runtime/network/tls/tls_buffer.h"
#include "core/memory/memory.h"
#include "platform/io/logger.h"

/// @brief Append data to the TLS buffer
/// @param data Span of data to append
/// @return The offset at which the data was appended, or -1 on allocation failure

INT32 TlsBuffer::Append(Span<const CHAR> data)
{
	auto r = CheckSize((INT32)data.Size());
	if (!r)
		return -1;
	Memory::Copy(buffer + size, data.Data(), data.Size());
	size += (INT32)data.Size();
	return size - (INT32)data.Size();
}

/// @brief Reserve space in the buffer without writing data
/// @param count Number of bytes to reserve
/// @return The offset at which the space was reserved, or -1 on allocation failure

INT32 TlsBuffer::AppendSize(INT32 count)
{
	auto r = CheckSize(count);
	if (!r)
		return -1;
	size += count;
	return size - count;
}

/// @brief Set the logical size of the TLS buffer
/// @param newSize The new size of the buffer
/// @return Result<void, Error>::Ok() on success, or TlsBuffer_AllocationFailed on failure

Result<void, Error> TlsBuffer::SetSize(INT32 newSize)
{
	size = 0;
	auto r = CheckSize(newSize);
	if (!r)
		return Result<void, Error>::Err(r.Error());
	size = newSize;
	return Result<void, Error>::Ok();
}

/// @brief Clean up the TLS buffer by zeroing and freeing memory if owned, resetting size and capacity

VOID TlsBuffer::Clear()
{
	if (buffer && ownsMemory)
	{
		Memory::Zero(buffer, capacity);
		delete[] buffer;
		buffer = nullptr;
	}
	size = 0;
	capacity = 0;
	readPos = 0;
}

/// @brief Ensure there is enough capacity in the TLS buffer to append additional data
/// @param appendSize The size of the data to be appended
/// @return Result<void, Error>::Ok() on success, or TlsBuffer_AllocationFailed on failure

Result<void, Error> TlsBuffer::CheckSize(INT32 appendSize)
{
	if (size + appendSize <= capacity)
	{
		LOG_DEBUG("Buffer size is sufficient: %d + %d <= %d", size, appendSize, capacity);
		return Result<void, Error>::Ok();
	}

	PCHAR oldBuffer = buffer;
	INT32 newLen = (size + appendSize) * 4;
	if (newLen < 256)
	{
		newLen = 256;
	}

	PCHAR newBuffer = (PCHAR)new CHAR[newLen];
	if (!newBuffer)
	{
		return Result<void, Error>::Err(Error::TlsBuffer_AllocationFailed);
	}
	if (size > 0)
	{
		LOG_DEBUG("Resizing buffer from %d to %d bytes", capacity, newLen);
		Memory::Copy(newBuffer, oldBuffer, size);
	}
	if (oldBuffer && ownsMemory)
	{
		Memory::Zero(oldBuffer, capacity);
		delete[] oldBuffer;
		oldBuffer = nullptr;
	}
	buffer = newBuffer;
	capacity = newLen;
	ownsMemory = true;
	return Result<void, Error>::Ok();
}

/// @brief Read a block of data from the TLS buffer into the provided span
/// @param buf Output span to read into

VOID TlsBuffer::Read(Span<CHAR> buf)
{
	Memory::Copy(buf.Data(), buffer + readPos, buf.Size());
	readPos += (INT32)buf.Size();
}

/// @brief Read a 24-bit big-endian unsigned integer from the TLS buffer
/// @return The 24-bit value read from the buffer

UINT32 TlsBuffer::ReadU24BE()
{
	UINT8 b0 = (UINT8)buffer[readPos];
	UINT8 b1 = (UINT8)buffer[readPos + 1];
	UINT8 b2 = (UINT8)buffer[readPos + 2];
	readPos += 3;
	return ((UINT32)b0 << 16) | ((UINT32)b1 << 8) | (UINT32)b2;
}
