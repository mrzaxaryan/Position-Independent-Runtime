/**
 * @file result.h
 * @brief Lightweight Result Type for Error Handling
 *
 * @details Tagged union `Result<T, E>` — either a success value (T) or error (E).
 *
 * @note Uses Clang builtin __is_trivially_destructible for zero-overhead destruction.
 *
 * @ingroup core
 *
 * @defgroup result Result Type
 * @ingroup core
 * @{
 */

#pragma once

#include "primitives.h"

/// Placement new for constructing objects in pre-allocated storage
FORCE_INLINE PVOID operator new(USIZE, PVOID ptr) noexcept { return ptr; }

template <typename T, typename E>
class [[nodiscard]] Result
{
	union { T m_value; E m_error; };
	BOOL m_isOk;

	FORCE_INLINE void DestroyActive() noexcept
	{
		if constexpr (!__is_trivially_destructible(T) || !__is_trivially_destructible(E))
		{
			if (m_isOk)
			{
				if constexpr (!__is_trivially_destructible(T))
					m_value.~T();
			}
			else
			{
				if constexpr (!__is_trivially_destructible(E))
					m_error.~E();
			}
		}
	}

public:
	using value_type = T;
	using error_type = E;

	[[nodiscard]] static FORCE_INLINE Result Ok(T value) noexcept
	{
		Result r;
		r.m_isOk = TRUE;
		new (&r.m_value) T(static_cast<T &&>(value));
		return r;
	}

	[[nodiscard]] static FORCE_INLINE Result Err(E error) noexcept
	{
		Result r;
		r.m_isOk = FALSE;
		new (&r.m_error) E(static_cast<E &&>(error));
		return r;
	}

	FORCE_INLINE ~Result() noexcept { DestroyActive(); }

	FORCE_INLINE Result(Result &&other) noexcept : m_isOk(other.m_isOk)
	{
		if (m_isOk)
			new (&m_value) T(static_cast<T &&>(other.m_value));
		else
			new (&m_error) E(static_cast<E &&>(other.m_error));
	}

	FORCE_INLINE Result &operator=(Result &&other) noexcept
	{
		if (this != &other)
		{
			DestroyActive();
			m_isOk = other.m_isOk;
			if (m_isOk)
				new (&m_value) T(static_cast<T &&>(other.m_value));
			else
				new (&m_error) E(static_cast<E &&>(other.m_error));
		}
		return *this;
	}

	Result(const Result &) = delete;
	Result &operator=(const Result &) = delete;

	[[nodiscard]] FORCE_INLINE BOOL IsOk() const noexcept { return m_isOk; }
	[[nodiscard]] FORCE_INLINE BOOL IsErr() const noexcept { return !m_isOk; }
	[[nodiscard]] FORCE_INLINE operator BOOL() const noexcept { return m_isOk; }

	[[nodiscard]] FORCE_INLINE T &Value() noexcept { return m_value; }
	[[nodiscard]] FORCE_INLINE const T &Value() const noexcept { return m_value; }
	[[nodiscard]] FORCE_INLINE E &Error() noexcept { return m_error; }
	[[nodiscard]] FORCE_INLINE const E &Error() const noexcept { return m_error; }

private:
	Result() noexcept {}
};

// Void specialization — operations with no return value
template <typename E>
class [[nodiscard]] Result<void, E>
{
	union { E m_error; };
	BOOL m_isOk;

	FORCE_INLINE void DestroyActive() noexcept
	{
		if constexpr (!__is_trivially_destructible(E))
		{
			if (!m_isOk)
				m_error.~E();
		}
	}

public:
	using value_type = void;
	using error_type = E;

	[[nodiscard]] static FORCE_INLINE Result Ok() noexcept
	{
		Result r;
		r.m_isOk = TRUE;
		return r;
	}

	[[nodiscard]] static FORCE_INLINE Result Err(E error) noexcept
	{
		Result r;
		r.m_isOk = FALSE;
		new (&r.m_error) E(static_cast<E &&>(error));
		return r;
	}

	FORCE_INLINE ~Result() noexcept { DestroyActive(); }

	FORCE_INLINE Result(Result &&other) noexcept : m_isOk(other.m_isOk)
	{
		if (!m_isOk)
			new (&m_error) E(static_cast<E &&>(other.m_error));
	}

	FORCE_INLINE Result &operator=(Result &&other) noexcept
	{
		if (this != &other)
		{
			DestroyActive();
			m_isOk = other.m_isOk;
			if (!m_isOk)
				new (&m_error) E(static_cast<E &&>(other.m_error));
		}
		return *this;
	}

	Result(const Result &) = delete;
	Result &operator=(const Result &) = delete;

	[[nodiscard]] FORCE_INLINE BOOL IsOk() const noexcept { return m_isOk; }
	[[nodiscard]] FORCE_INLINE BOOL IsErr() const noexcept { return !m_isOk; }
	[[nodiscard]] FORCE_INLINE operator BOOL() const noexcept { return m_isOk; }

	[[nodiscard]] FORCE_INLINE E &Error() noexcept { return m_error; }
	[[nodiscard]] FORCE_INLINE const E &Error() const noexcept { return m_error; }

private:
	Result() noexcept {}
};

/** @} */ // end of result group
