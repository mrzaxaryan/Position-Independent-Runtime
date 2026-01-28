#pragma once

#include "primitives.h"
#include "string.h"
#include "memory.h"
#include "socket.h"
#include "tls.h"
#include "ip_address.h"

class HttpClient
{
private:
    BOOL isSecure;
    CHAR hostName[1024];
    CHAR path[1024];
    IPAddress ipAddress;
    UINT16 port;
    TLSClient tlsContext;
    Socket socketContext;

public:
    HttpClient(PCCHAR url, PCCHAR ipAddress);
    HttpClient(PCCHAR url);

    BOOL Open();
    BOOL Close();
    SSIZE Read(PVOID buffer, UINT32 bufferLength);
    UINT32 Write(PCVOID buffer, UINT32 bufferLength);

    BOOL SendGetRequest();
    BOOL SendPostRequest(PCVOID data, UINT32 dataLength);

    ~HttpClient();

    static BOOL ParseUrl(PCCHAR url, PCHAR host, PCHAR path, PUINT16 port, PBOOL secure);
};
