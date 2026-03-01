#pragma once

/**
 * @file tls_buffer.h
 * @brief Unified TLS buffer for reading and writing handshake and record data
 *
 * @details Provides a dynamically-growing byte buffer used throughout the TLS
 * implementation for constructing handshake messages, buffering received records,
 * and managing the application data channel. Supports both owned (write mode) and
 * non-owned (read mode) memory, with typed read/write operations for building
 * and parsing TLS wire-format structures.
 */

#include "core/core.h"

/// Unified TLS buffer for both reading and writing
class TlsBuffer
{
private:
	PCHAR buffer;
	INT32 capacity;
	INT32 size;
	INT32 readPos;
	BOOL ownsMemory;

public:
	// Default constructor - owns memory, write mode
	constexpr TlsBuffer() : buffer(nullptr), capacity(0), size(0), readPos(0), ownsMemory(true) {}

	// Constructor for wrapping existing data - read mode (does not own memory)
	constexpr TlsBuffer(Span<CHAR> data) : buffer(data.Data()), capacity((INT32)data.Size()), size((INT32)data.Size()), readPos(0), ownsMemory(false) {}

	~TlsBuffer()
	{
		if (ownsMemory)
			Clear();
	}

	TlsBuffer(const TlsBuffer &) = delete;
	TlsBuffer &operator=(const TlsBuffer &) = delete;

	TlsBuffer(TlsBuffer &&other) noexcept
		: buffer(other.buffer), capacity(other.capacity), size(other.size), readPos(other.readPos), ownsMemory(other.ownsMemory)
	{
		other.buffer = nullptr;
		other.capacity = 0;
		other.size = 0;
		other.readPos = 0;
		other.ownsMemory = false;
	}
	TlsBuffer &operator=(TlsBuffer &&other) noexcept
	{
		if (this != &other)
		{
			if (ownsMemory)
				Clear();
			buffer = other.buffer;
			capacity = other.capacity;
			size = other.size;
			readPos = other.readPos;
			ownsMemory = other.ownsMemory;
			other.buffer = nullptr;
			other.capacity = 0;
			other.size = 0;
			other.readPos = 0;
			other.ownsMemory = false;
		}
		return *this;
	}

	/**
	 * @brief Appends raw byte data to the buffer
	 * @param data Span of data to append
	 * @return Offset in the buffer where the data was written
	 */
	INT32 Append(Span<const CHAR> data);

	/**
	 * @brief Appends a typed value to the buffer in native byte order
	 * @tparam T The type of value to append
	 * @param data The value to append
	 * @return Offset in the buffer where the value was written
	 */
	template <typename T>
	INT32 Append(T data)
	{
		CheckSize(sizeof(T));
		*(T *)(buffer + this->size) = data;
		this->size += sizeof(T);
		return this->size - sizeof(T);
	}

	/**
	 * @brief Reserves space in the buffer without writing data
	 * @param size Number of bytes to reserve
	 * @return Offset where the reserved space begins
	 */
	INT32 AppendSize(INT32 size);

	/**
	 * @brief Sets the logical size of the buffer
	 * @param size New size in bytes
	 */
	VOID SetSize(INT32 size);

	/**
	 * @brief Frees the buffer memory and resets state
	 */
	VOID Clear();

	/**
	 * @brief Ensures the buffer has capacity for additional data
	 * @param appendSize Number of additional bytes needed
	 */
	VOID CheckSize(INT32 appendSize);

	/**
	 * @brief Reads a typed value from the buffer at the current read position
	 * @tparam T The type of value to read
	 * @return The value read from the buffer
	 */
	template <typename T>
	T Read();

	/**
	 * @brief Reads raw bytes from the buffer into the provided span
	 * @param buf Output span to read into
	 */
	VOID Read(Span<CHAR> buf);

	/**
	 * @brief Reads a 24-bit big-endian unsigned integer from the buffer
	 * @return The 24-bit value as a UINT32
	 */
	UINT32 ReadU24BE();

	// Accessors
	constexpr INT32 GetSize() const { return size; }
	constexpr PCHAR GetBuffer() const { return buffer; }
	constexpr VOID SetBuffer(PCHAR buf)
	{
		buffer = buf;
		if (!ownsMemory)
			size = 0;
	}
	constexpr INT32 GetReadPosition() const { return readPos; }
	constexpr VOID AdvanceReadPosition(INT32 sz) { readPos += sz; }
	constexpr VOID ResetReadPos() { readPos = 0; }
};
