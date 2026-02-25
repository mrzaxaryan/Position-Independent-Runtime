#include "socket.h"
#include "syscall.h"
#include "system.h"
#include "memory.h"
#include "ip_address.h"

// BSD socket addresses have a sa_len field that must be set
// macOS AF_INET6 = 30 (different from Linux 10 and Windows 23)
// These are handled via platform-conditional defines in socket.h

Socket::Socket(const IPAddress &ipAddress, UINT16 port)
	: ip(ipAddress), port(port), m_socket(nullptr)
{
	SSIZE fd = System::Call(SYS_SOCKET, SocketAddressHelper::GetAddressFamily(ip), SOCK_STREAM, IPPROTO_TCP);
	if (fd < 0)
	{
		m_socket = (PVOID)INVALID_FD;
		return;
	}
	m_socket = (PVOID)fd;
}

Result<void, SocketError> Socket::Bind(SockAddr &socketAddress, INT32 shareType)
{
	(VOID)shareType; // not used on macOS

	if (!IsValid())
		return Result<void, SocketError>::Err(SocketError{SocketError::BindFailed_Bind, 0});

	SSIZE  sockfd  = (SSIZE)m_socket;
	UINT32 addrLen = (socketAddress.sin_family == AF_INET6) ? sizeof(SockAddr6) : sizeof(SockAddr);
	SSIZE  result  = System::Call(SYS_BIND, sockfd, (USIZE)&socketAddress, addrLen);
	if (result != 0)
		return Result<void, SocketError>::Err(SocketError{SocketError::BindFailed_Bind, (UINT32)(-result)});

	return Result<void, SocketError>::Ok();
}

Result<void, SocketError> Socket::Open()
{
	if (!IsValid())
		return Result<void, SocketError>::Err(SocketError{SocketError::OpenFailed_HandleInvalid, 0});

	SSIZE sockfd = (SSIZE)m_socket;

	union
	{
		SockAddr  addr4;
		SockAddr6 addr6;
	} addrBuffer;

	UINT32 addrLen = SocketAddressHelper::PrepareAddress(ip, port, &addrBuffer, sizeof(addrBuffer));
	if (addrLen == 0)
		return Result<void, SocketError>::Err(SocketError{SocketError::OpenFailed_Connect, 0});

	SSIZE result = System::Call(SYS_CONNECT, sockfd, (USIZE)&addrBuffer, addrLen);
	if (result != 0)
		return Result<void, SocketError>::Err(SocketError{SocketError::OpenFailed_Connect, (UINT32)(-result)});

	return Result<void, SocketError>::Ok();
}

Result<void, SocketError> Socket::Close()
{
	if (!IsValid())
		return Result<void, SocketError>::Err(SocketError{SocketError::CloseFailed_Close, 0});

	SSIZE sockfd = (SSIZE)m_socket;
	System::Call(SYS_CLOSE, sockfd);
	m_socket = (PVOID)INVALID_FD;
	return Result<void, SocketError>::Ok();
}

Result<SSIZE, SocketError> Socket::Read(PVOID buffer, UINT32 bufferLength)
{
	if (!IsValid())
		return Result<SSIZE, SocketError>::Err(SocketError{SocketError::ReadFailed_HandleInvalid, 0});

	SSIZE sockfd = (SSIZE)m_socket;
	SSIZE result = System::Call(SYS_RECVFROM, sockfd, (USIZE)buffer, bufferLength, 0, 0, 0);
	if (result < 0)
		return Result<SSIZE, SocketError>::Err(SocketError{SocketError::ReadFailed_Recv, (UINT32)(-result)});

	return Result<SSIZE, SocketError>::Ok(result);
}

Result<UINT32, SocketError> Socket::Write(PCVOID buffer, UINT32 bufferLength)
{
	if (!IsValid())
		return Result<UINT32, SocketError>::Err(SocketError{SocketError::WriteFailed_HandleInvalid, 0});

	SSIZE  sockfd    = (SSIZE)m_socket;
	UINT32 totalSent = 0;

	while (totalSent < bufferLength)
	{
		SSIZE sent = System::Call(SYS_SENDTO, sockfd,
		                          (USIZE)((const CHAR *)buffer + totalSent),
		                          bufferLength - totalSent, 0, 0, 0);
		if (sent <= 0)
			return Result<UINT32, SocketError>::Err(SocketError{SocketError::WriteFailed_Send, sent < 0 ? (UINT32)(-sent) : 0});

		totalSent += (UINT32)sent;
	}

	return Result<UINT32, SocketError>::Ok(totalSent);
}
