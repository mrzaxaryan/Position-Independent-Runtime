#pragma once

#include "primitives.h"

/**
 * @brief Non-owning view over a contiguous sequence of elements
 *
 * @tparam T Element type (can be const-qualified for read-only views)
 *
 * @details Span is a lightweight, non-owning reference to a contiguous region
 * of memory. It stores a pointer and a size, replacing the common (pointer, size)
 * parameter pair pattern throughout the codebase.
 *
 * Span<T> implicitly converts to Span<const T>, enabling functions that accept
 * Span<const T> to receive Span<T> arguments without explicit casting.
 *
 * @par Example Usage:
 * @code
 * UINT8 buffer[64];
 * Span<UINT8> writable(buffer);           // From array (size deduced)
 * Span<const UINT8> readable = writable;  // Implicit const conversion
 *
 * Span<UINT8> sub = writable.Subspan(4, 16);  // View of bytes [4..20)
 * @endcode
 */
template <typename T>
class Span
{
private:
	T *m_data;
	USIZE m_size;

public:
	constexpr Span() : m_data(nullptr), m_size(0) {}

	constexpr Span(T *data, USIZE size) : m_data(data), m_size(size) {}

	template <USIZE N>
	constexpr Span(T (&arr)[N]) : m_data(arr), m_size(N) {}

	// Span<T> -> Span<const T> implicit conversion
	template <typename U>
		requires(__is_same_as(T, const U))
	constexpr Span(const Span<U> &other) : m_data(other.Data()), m_size(other.Size()) {}

	constexpr T *Data() const { return m_data; }
	constexpr USIZE Size() const { return m_size; }
	constexpr USIZE SizeBytes() const { return m_size * sizeof(T); }
	constexpr BOOL IsEmpty() const { return m_size == 0; }

	constexpr T &operator[](USIZE index) const { return m_data[index]; }

	constexpr Span Subspan(USIZE offset) const { return Span(m_data + offset, m_size - offset); }
	constexpr Span Subspan(USIZE offset, USIZE count) const { return Span(m_data + offset, count); }
	constexpr Span First(USIZE count) const { return Span(m_data, count); }
	constexpr Span Last(USIZE count) const { return Span(m_data + m_size - count, count); }

	constexpr T *begin() const { return m_data; }
	constexpr T *end() const { return m_data + m_size; }

	// Stack-only
	VOID *operator new(USIZE) = delete;
	VOID operator delete(VOID *) = delete;
	VOID *operator new(USIZE, PVOID ptr) noexcept { return ptr; }
};
