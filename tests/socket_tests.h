#pragma once

#include "runtime.h"
#include "socket.h"
#include "logger.h"

// =============================================================================
// Socket Tests - AFD Socket Implementation Validation
// =============================================================================

namespace SocketTests
{
	// Test 1: Socket creation
	BOOL TestSocketCreation()
	{
		LOG_INFO("Test: Socket Creation");

		// Create socket for connection to 79.133.51.99:80
		Socket sock(0x6333854F, 80); // IP in network byte order (79.133.51.99)

		if (!sock.IsValid())
		{
			LOG_ERROR("Socket creation failed");
			return FALSE;
		}

		LOG_INFO("Socket created successfully");
		sock.Close();
		return TRUE;
	}

	// Test 2: Socket connection
	BOOL TestSocketConnection()
	{
		LOG_INFO("Test: Socket Connection");

		// Create socket and connect to 79.133.51.99:80
		Socket sock(0x6333854F, 80);

		if (!sock.IsValid())
		{
			LOG_ERROR("Socket creation failed");
			return FALSE;
		}

		if (!sock.Open())
		{
			LOG_ERROR("Socket connection failed");
			sock.Close();
			return FALSE;
		}

		LOG_INFO("Socket connected successfully");
		sock.Close();
		return TRUE;
	}

	// Test 3: HTTP GET request
	BOOL TestHttpRequest()
	{
		LOG_INFO("Test: HTTP GET Request");

		// Create socket and connect to 79.133.51.99:80
		Socket sock(0x6333854F, 80);

		if (!sock.IsValid() || !sock.Open())
		{
			LOG_ERROR("Socket initialization or connection failed");
			return FALSE;
		}

		// Send HTTP GET request
		auto request = "GET / HTTP/1.1\r\nHost: 0y.wtf\r\nConnection: close\r\n\r\n"_embed;
		UINT32 bytesSent = sock.Write((PCVOID)(PCCHAR)request, request.Length);

		if (bytesSent != request.Length)
		{
			LOG_ERROR("Failed to send complete HTTP request (sent %d/%d bytes)", bytesSent, request.Length);
			sock.Close();
			return FALSE;
		}

		LOG_INFO("HTTP request sent successfully (%d bytes)", bytesSent);

		// Receive response
		CHAR buffer[512];
		Memory::Zero(buffer, sizeof(buffer));
		SSIZE bytesRead = sock.Read(buffer, sizeof(buffer) - 1);

		if (bytesRead <= 0)
		{
			LOG_ERROR("Failed to receive HTTP response");
			sock.Close();
			return FALSE;
		}

		LOG_INFO("Received HTTP response (%d bytes)", bytesRead);
		LOG_DEBUG("Response preview: %.100s...", buffer);

		sock.Close();
		return TRUE;
	}

	// Run all socket tests
	VOID RunAllTests()
	{
		LOG_INFO("=== Starting Socket Tests ===");

		UINT32 passed = 0;
		UINT32 total = 3;

		if (TestSocketCreation())
			passed++;
		if (TestSocketConnection())
			passed++;
		if (TestHttpRequest())
			passed++;

		LOG_INFO("=== Socket Tests Complete: %d/%d passed ===", passed, total);
	}
}
