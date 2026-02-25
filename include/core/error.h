#pragma once

#include "primitives.h"
#include "memory.h"

// Unified error — all network/platform layers push codes onto a call-stack array.
// Each layer appends its code after any codes pushed by lower layers.
struct Error
{
	// PIR runtime failure points — one unique value per failure site.
	// OS error codes (NTSTATUS, errno, EFI_STATUS) are stored directly in
	// ErrorCode.Code when Platform != Runtime; they are not listed here.
	enum ErrorCodes : UINT32
	{
		None = 0, // no error / empty slot

		// -------------------------
		// Socket errors (1–15)
		// -------------------------
		Socket_CreateFailed_Open = 1,		   // ZwCreateFile / socket() failed
		Socket_BindFailed_EventCreate = 2,	   // ZwCreateEvent failed (Windows only)
		Socket_BindFailed_Bind = 3,			   // AFD_BIND / bind() syscall failed
		Socket_OpenFailed_HandleInvalid = 4,   // socket was never created successfully
		Socket_OpenFailed_EventCreate = 5,	   // ZwCreateEvent failed (Windows only)
		Socket_OpenFailed_Connect = 6,		   // AFD_CONNECT / connect() syscall failed
		Socket_CloseFailed_Close = 7,		   // ZwClose / close() failed
		Socket_ReadFailed_HandleInvalid = 8,   // socket handle invalid
		Socket_ReadFailed_EventCreate = 9,	   // ZwCreateEvent failed (Windows only)
		Socket_ReadFailed_Timeout = 10,		   // receive timed out
		Socket_ReadFailed_Recv = 11,		   // AFD_RECV / recv() syscall failed
		Socket_WriteFailed_HandleInvalid = 12, // socket handle invalid
		Socket_WriteFailed_EventCreate = 13,   // ZwCreateEvent failed (Windows only)
		Socket_WriteFailed_Timeout = 14,	   // send timed out
		Socket_WriteFailed_Send = 15,		   // AFD_SEND / send() syscall failed

		// -------------------------
		// TLS errors (16–22)
		// -------------------------
		Tls_OpenFailed_Socket = 16,	   // underlying socket Open() failed
		Tls_OpenFailed_Handshake = 17, // TLS handshake failed
		Tls_CloseFailed_Socket = 18,   // underlying socket Close() failed
		Tls_ReadFailed_NotReady = 19,  // connection not established
		Tls_ReadFailed_Receive = 20,   // ProcessReceive() failed
		Tls_WriteFailed_NotReady = 21, // connection not established
		Tls_WriteFailed_Send = 22,	   // SendPacket() failed

		// -------------------------
		// WebSocket errors (23–32)
		// -------------------------
		Ws_TransportFailed = 23,  // TLS/socket transport open failed
		Ws_DnsFailed = 24,		  // DNS resolution failed
		Ws_HandshakeFailed = 25,  // HTTP 101 upgrade handshake failed
		Ws_WriteFailed = 26,	  // frame write to transport failed
		Ws_NotConnected = 27,	  // operation attempted on closed connection
		Ws_AllocFailed = 28,	  // memory allocation failed
		Ws_ReceiveFailed = 29,	  // frame receive failed
		Ws_ConnectionClosed = 30, // server sent CLOSE frame
		Ws_InvalidFrame = 31,	  // received frame with invalid RSV bits or opcode
		Ws_FrameTooLarge = 32,	  // received frame exceeds size limit
	};

	// Which OS layer an ErrorCode entry came from.
	// When Platform != Runtime, Code holds the raw OS error value.
	enum class PlatformKind : UINT8
	{
		Runtime = 0, // PIR runtime layer — Code is an ErrorCodes enumerator
		Windows = 1, // NTSTATUS  — Code holds the raw NTSTATUS value
		Posix   = 2, // errno     — Code holds errno as a positive UINT32
		Uefi    = 3, // EFI_STATUS — Code holds the raw EFI_STATUS value
	};

	// A single entry pushed onto the Error call-stack.
	// When Platform == Runtime: Code is one of the ErrorCodes enumerators above.
	// When Platform != Runtime: Code holds the raw OS error value cast to UINT32.
	struct ErrorCode
	{
		ErrorCodes   Code;
		PlatformKind Platform;

		ErrorCode(UINT32 code = 0, PlatformKind platform = PlatformKind::Runtime)
			: Code((ErrorCodes)code), Platform(platform)
		{
		}
	};

	static constexpr UINT32 MaxDepth = 8;

	UINT32    m_depth;				 // push count; may exceed MaxDepth on overflow
	ErrorCode RuntimeCode[MaxDepth]; // call stack, innermost first; None = empty slot

	Error() { Memory::Zero(this, sizeof(Error)); }

	// -- Static factory --

	// Build an Error from one or more codes (innermost first).
	// For OS failures: pass ErrorCode((UINT32)status, PlatformKind::Windows/Posix/Uefi) as the first arg.
	// For guard failures: pass the ErrorCodes enumerator directly (Platform defaults to Runtime).
	// Multiple codes: Error::FromCode(osCode, Socket_Xxx) — no Push() needed.
	template <typename... Codes>
	[[nodiscard]] static Error FromCode(Codes... codes)
	{
		Error err;
		(err.Push(codes), ...);
		return err;
	}

	// -- Mutation --

	// Push a code onto the call stack (innermost layer pushes first).
	// Codes pushed past MaxDepth are counted but not stored; check Overflow() to detect.
	// Returns *this to enable chaining: err.Push(A).Push(B)
	Error& Push(ErrorCode code)
	{
		if (m_depth < MaxDepth)
			RuntimeCode[m_depth] = code;
		m_depth++;
		return *this;
	}

	// -- Queries --

	// Returns the OS kind from the innermost (bottom) code's Platform field.
	// Meaningful only when !IsEmpty(). Returns PlatformKind::Runtime for guard failures.
	[[nodiscard]] PlatformKind Kind()     const { return Bottom().Platform; }
	[[nodiscard]] UINT32       Depth()    const { return m_depth; }
	[[nodiscard]] BOOL         IsEmpty()  const { return m_depth == 0; }
	[[nodiscard]] BOOL         Overflow() const { return m_depth > MaxDepth; }

	// Returns the innermost (first pushed, lowest-layer) code, or None if empty.
	[[nodiscard]] ErrorCode Bottom() const { return m_depth > 0 ? RuntimeCode[0] : ErrorCode(); }

	// Returns the outermost (last pushed, highest-layer) code, or None if empty.
	[[nodiscard]] ErrorCode Top() const
	{
		if (m_depth == 0)
			return ErrorCode();
		UINT32 stored = m_depth < MaxDepth ? m_depth : MaxDepth;
		return RuntimeCode[stored - 1];
	}

	// Returns true if any stored code matches the given ErrorCodes enumerator.
	// Only meaningful for Runtime-platform entries; OS error values are not enumerated.
	[[nodiscard]] BOOL HasCode(ErrorCodes code) const
	{
		UINT32 stored = m_depth < MaxDepth ? m_depth : MaxDepth;
		for (UINT32 i = 0; i < stored; i++)
		{
			if (RuntimeCode[i].Code == code)
				return true;
		}
		return false;
	}

	// Returns the code at position index (0 = innermost). Returns None if out of range.
	[[nodiscard]] ErrorCode operator[](UINT32 index) const
	{
		return (index < MaxDepth && index < m_depth) ? RuntimeCode[index] : ErrorCode();
	}
};
