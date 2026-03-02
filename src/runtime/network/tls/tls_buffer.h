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
	 * @return Offset in the buffer where the data was written, or -1 on allocation failure
	 */
	INT32 Append(Span<const CHAR> data);

	/**
	 * @brief Appends a typed value to the buffer in native byte order
	 * @tparam T The type of value to append
	 * @param value The value to append
	 * @return Offset in the buffer where the value was written, or -1 on allocation failure
	 */
	template <typename T>
	INT32 Append(T value)
	{
		if (!CheckSize(sizeof(T)))
			return -1;
		Memory::Copy(buffer + size, &value, sizeof(T));
		size += sizeof(T);
		return size - sizeof(T);
	}

	/**
	 * @brief Reserves space in the buffer without writing data
	 * @param count Number of bytes to reserve
	 * @return Offset where the reserved space begins, or -1 on allocation failure
	 */
	INT32 AppendSize(INT32 count);

	/**
	 * @brief Sets the logical size of the buffer
	 * @param newSize New size in bytes
	 */
	VOID SetSize(INT32 newSize);

	/**
	 * @brief Frees the buffer memory and resets state
	 */
	VOID Clear();

	/**
	 * @brief Ensures the buffer has capacity for additional data
	 * @param appendSize Number of additional bytes needed
	 * @return true if capacity is sufficient (or was grown), false on allocation failure
	 */
	[[nodiscard]] BOOL CheckSize(INT32 appendSize);

	/**
	 * @brief Reads a typed value from the buffer at the current read position
	 * @tparam T The type of value to read
	 * @return The value read from the buffer
	 */
	template <typename T>
	T Read()
	{
		T value;
		Memory::Copy(&value, buffer + readPos, sizeof(T));
		readPos += sizeof(T);
		return value;
	}

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

	/**
	 * @brief Writes a 16-bit value in big-endian byte order at the specified offset
	 * @param offset Byte offset within the buffer to write at
	 * @param value The 16-bit value to write
	 *
	 * @details Uses byte-by-byte writes to avoid unaligned access faults on ARM.
	 */
	constexpr VOID PatchU16BE(INT32 offset, UINT16 value)
	{
		PUCHAR p = (PUCHAR)(buffer + offset);
		p[0] = (UINT8)(value >> 8);
		p[1] = (UINT8)(value & 0xFF);
	}

	/**
	 * @brief Writes a 24-bit value in big-endian byte order at the specified offset
	 * @param offset Byte offset within the buffer to write at
	 * @param value The value to write (lower 24 bits used)
	 *
	 * @details Uses byte-by-byte writes to avoid unaligned access faults on ARM.
	 */
	constexpr VOID PatchU24BE(INT32 offset, UINT32 value)
	{
		PUCHAR p = (PUCHAR)(buffer + offset);
		p[0] = (UINT8)((value >> 16) & 0xFF);
		p[1] = (UINT8)((value >> 8) & 0xFF);
		p[2] = (UINT8)(value & 0xFF);
	}

	/**
	 * @brief Returns the buffer contents as a read-only span
	 * @return Span wrapping [GetBuffer(), GetSize())
	 */
	constexpr Span<const CHAR> AsSpan() const { return Span<const CHAR>(buffer, (USIZE)size); }

	/**
	 * @brief Returns the buffer contents as a writable span
	 * @return Span wrapping [GetBuffer(), GetSize())
	 */
	constexpr Span<CHAR> AsWritableSpan() { return Span<CHAR>(buffer, (USIZE)size); }

	/// @name Accessors
	/// @{

	/** @brief Get the current logical size in bytes */
	constexpr INT32 GetSize() const { return size; }

	/** @brief Get the underlying buffer pointer */
	constexpr PCHAR GetBuffer() const { return buffer; }

	/** @brief Set the underlying buffer pointer (non-owned buffers only) */
	constexpr VOID SetBuffer(PCHAR buf)
	{
		buffer = buf;
		if (!ownsMemory)
			size = 0;
	}

	/** @brief Get the current read position */
	constexpr INT32 GetReadPosition() const { return readPos; }

	/** @brief Advance the read position by the specified number of bytes */
	constexpr VOID AdvanceReadPosition(INT32 sz) { readPos += sz; }

	/** @brief Reset the read position to the start of the buffer */
	constexpr VOID ResetReadPos() { readPos = 0; }

	/// @}
};
