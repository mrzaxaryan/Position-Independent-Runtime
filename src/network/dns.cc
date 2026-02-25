#include "dns.h"
#include "http.h"
#include "logger.h"
#include "memory.h"
#include "string.h"
#include "tls.h"
#include "embedded_string.h"

// https://tools.ietf.org/html/rfc1035#section-4.1.1
// Note: z often is shown as of size 3, but it is actually separated into 3 fields of 1 bit each - ad, cd
typedef struct DNS_REQUEST_HEADER
{
    UINT16 id;        // identification number
    UCHAR rd : 1;     // recursion desired
    UCHAR tc : 1;     // truncated message
    UCHAR aa : 1;     // authoritive answer
    UCHAR opcode : 4; // purpose of message
    UCHAR qr : 1;     // query/response flag

    UCHAR rcode : 4; // response code
    UCHAR cd : 1;    // checking disabled
    UCHAR ad : 1;    // authenticated data
    UCHAR z : 1;     // reserved
    UCHAR ra : 1;    // recursion available

    UINT16 qCount;    // number of question entries
    UINT16 ansCount;  // number of answer entries
    UINT16 authCount; // number of authority entries
    UINT16 addCount;  // number of resource entries
} DNS_REQUEST_HEADER, *PDNS_REQUEST_HEADER;

typedef struct DNS_REQUEST_QUESTION
{
    UINT16 qtype;  // type of the query
    UINT16 qclass; // class of the query
} DNS_REQUEST_QUESTION, *PDNS_REQUEST_QUESTION;

static inline BOOL IsLocalhost(PCCHAR host, IPAddress &result, RequestType type)
{
    auto localhost = "localhost"_embed;
    if (String::Compare(host, (PCCHAR)localhost))
    {
        result = IPAddress::LocalHost(type == AAAA);
        return TRUE;
    }
    return FALSE;
}

static inline UINT16 ReadU16BE(PCVOID buffer, USIZE index)
{
    const UINT8 *p = (const UINT8 *)buffer;
    return (UINT16)((p[index] << 8) | p[index + 1]);
}


static INT32 SkipName(PUINT8 ptr)
{
    PUINT8 p = ptr;
    while (p)
    {
        UINT8 label = *p;

        if (label == 0)
            return (INT32)(p - ptr + 1);

        if (label >= 0xC0)
            return (INT32)(p - ptr + 2);

        if (label > 63)
        {
            LOG_WARNING("SkipName: invalid label length: %d", label);
            return -1;
        }

        p += label + 1;
    }
    return -1;
}

// Parse DNS answer section, extract A/AAAA address. Sets parsedLen to bytes consumed.
static BOOL ParseAnswer(PUINT8 ptr, INT32 cnt, IPAddress &ipAddress, INT32 &parsedLen)
{
    INT32 len = 0;
    while (cnt > 0)
    {
        PUINT8 p = ptr + len;

        INT32 nameLen = SkipName(p);
        if (nameLen <= 0)
        {
            LOG_WARNING("ParseAnswer: failed to skip answer name");
            break;
        }

        // Fixed fields: type(2) + class(2) + ttl(4) + rdlength(2) = 10 bytes
        PUINT8 fixedFields = p + nameLen;
        UINT16 type = ReadU16BE(fixedFields, 0);
        UINT16 rdlength = ReadU16BE(fixedFields, 8);
        PUINT8 rdata = fixedFields + 10;

        if (type == A && rdlength == 4)
        {
            ipAddress = IPAddress::FromIPv4(*(PUINT32)rdata);
            parsedLen = len + nameLen + 10 + rdlength;
            return TRUE;
        }
        else if (type == AAAA && rdlength == 16)
        {
            ipAddress = IPAddress::FromIPv6(rdata);
            parsedLen = len + nameLen + 10 + rdlength;
            return TRUE;
        }

        len += nameLen + 10 + rdlength;
        cnt--;
    }

    parsedLen = len;
    return FALSE;
}

static INT32 ParseQuery(PUINT8 ptr, INT32 cnt)
{
    PUINT8 p = ptr;
    while (cnt > 0)
    {
        INT32 nameLen = SkipName(p);
        if (nameLen <= 0)
        {
            LOG_WARNING("ParseQuery: invalid name length");
            return -1;
        }
        p += nameLen + sizeof(DNS_REQUEST_QUESTION);
        cnt--;
    }
    return (INT32)(p - ptr);
}

static BOOL ParseDnsResponse(PUINT8 buffer, UINT16 len, IPAddress &ipAddress)
{
    if (!buffer || len < sizeof(DNS_REQUEST_HEADER))
    {
        LOG_WARNING("ParseDnsResponse: invalid parameters");
        return FALSE;
    }

    UINT16 flags = ReadU16BE(buffer, 2);
    if (!(flags & 0x8000))
    {
        LOG_WARNING("ParseDnsResponse: not a response");
        return FALSE;
    }

    UINT16 qCount = ReadU16BE(buffer, 4);
    UINT16 ansCount = ReadU16BE(buffer, 6);

    if (ansCount == 0 || ansCount > 20)
    {
        LOG_WARNING("ParseDnsResponse: invalid answer count: %d", ansCount);
        return FALSE;
    }

    INT32 recordOffset = sizeof(DNS_REQUEST_HEADER);

    if (qCount > 0)
    {
        INT32 size = ParseQuery(buffer + recordOffset, qCount);
        if (size <= 0)
        {
            LOG_WARNING("ParseDnsResponse: invalid query size: %d", size);
            return FALSE;
        }
        recordOffset += size;
    }

    INT32 parsedLen = 0;
    return ParseAnswer(buffer + recordOffset, ansCount, ipAddress, parsedLen);
}

// Convert hostname to DNS wire format (length-prefixed labels)
static VOID FormatDnsName(PUINT8 dns, PCCHAR host)
{
    if (!dns || !host)
        return;

    UINT32 i, t = 0;
    USIZE hostLen = String::Length(host);
    for (i = 0; i < (UINT32)hostLen; i++)
    {
        if (host[i] == '.')
        {
            *dns++ = i - t;
            for (; t < i; t++)
                *dns++ = host[t];
            t++;
        }
    }
    if (hostLen > 0 && host[hostLen - 1] != '.')
    {
        *dns++ = i - t;
        for (; t < i; t++)
            *dns++ = host[t];
    }
    *dns = '\0';
}

// Generate a DNS-over-HTTPS query packet (no TCP length prefix)
static UINT32 GenerateQuery(PCCHAR host, RequestType dnstype, PCHAR buffer)
{
    PDNS_REQUEST_HEADER pHeader = (PDNS_REQUEST_HEADER)buffer;

    pHeader->id = (UINT16)0x24a1;
    pHeader->qr = 0;
    pHeader->opcode = 0;
    pHeader->aa = 0;
    pHeader->tc = 0;
    pHeader->rd = 1;
    pHeader->ra = 0;
    pHeader->z = 0;
    pHeader->ad = 0;
    pHeader->cd = 0;
    pHeader->rcode = 0;
    pHeader->qCount = UINT16SwapByteOrder(1);
    pHeader->ansCount = 0;
    pHeader->authCount = 0;
    pHeader->addCount = 0;

    PCHAR qname = (PCHAR)pHeader + sizeof(DNS_REQUEST_HEADER);
    FormatDnsName((PUINT8)qname, host);

    PCHAR pCurrent = qname;
    while (*pCurrent != 0)
        pCurrent += (*pCurrent + 1);
    pCurrent++;

    PDNS_REQUEST_QUESTION pQuestion = (PDNS_REQUEST_QUESTION)pCurrent;
    pQuestion->qclass = UINT16SwapByteOrder(1);
    pQuestion->qtype = UINT16SwapByteOrder(dnstype);

    return (UINT32)((PCHAR)pQuestion + sizeof(DNS_REQUEST_QUESTION) - buffer);
}

IPAddress DNS::ResolveOverHttp(PCCHAR host, const IPAddress &DNSServerIp, PCCHAR DNSServerName, RequestType dnstype)
{
    IPAddress result;
    if (IsLocalhost(host, result, dnstype))
        return result;

    TLSClient tlsClient(DNSServerName, DNSServerIp, 443);
    if (!tlsClient.Open())
    {
        LOG_WARNING("Failed to connect to DNS server");
        return IPAddress::Invalid();
    }

    UINT8 queryBuffer[256];
    UINT32 querySize = GenerateQuery(host, dnstype, (PCHAR)queryBuffer);

    auto writeStr = [&tlsClient](PCCHAR s) { tlsClient.Write(s, String::Length(s)); };

    CHAR sizeBuf[8];
    String::UIntToStr(querySize, sizeBuf, sizeof(sizeBuf));

    writeStr("POST /dns-query HTTP/1.1\r\nHost: "_embed);
    writeStr(DNSServerName);
    writeStr("\r\nContent-Type: application/dns-message\r\nAccept: application/dns-message\r\nContent-Length: "_embed);
    writeStr(sizeBuf);
    writeStr("\r\n\r\n"_embed);

    tlsClient.Write(queryBuffer, querySize);

    INT64 contentLength = -1;

    if (!HttpClient::ReadResponseHeaders(tlsClient, 200, contentLength))
    {
        LOG_WARNING("DNS server returned non-200 response");
        return IPAddress::Invalid();
    }

    if (contentLength <= 0 || contentLength > 512)
    {
        LOG_WARNING("Invalid or missing Content-Length header");
        return IPAddress::Invalid();
    }

    CHAR binaryResponse[512];
    UINT32 totalRead = 0;
    while (totalRead < (UINT32)contentLength)
    {
        SSIZE bytesRead = tlsClient.Read(binaryResponse + totalRead, (UINT32)contentLength - totalRead);
        if (bytesRead <= 0)
        {
            LOG_WARNING("Failed to read DNS binary response");
            return IPAddress::Invalid();
        }
        totalRead += (UINT32)bytesRead;
    }

    IPAddress ipAddress;
    if (!ParseDnsResponse((PUINT8)binaryResponse, (UINT16)contentLength, ipAddress))
    {
        LOG_WARNING("Failed to parse DNS response");
        return IPAddress::Invalid();
    }

    return ipAddress;
}

IPAddress DNS::CloudflareResolve(PCCHAR host, RequestType dnstype)
{
    auto serverName = "one.one.one.one"_embed;
    IPAddress ip = ResolveOverHttp(host, IPAddress::FromIPv4(0x01010101), (PCCHAR)serverName, dnstype);
    if (!ip.IsValid())
        ip = ResolveOverHttp(host, IPAddress::FromIPv4(0x01000001), (PCCHAR)serverName, dnstype);
    return ip;
}

IPAddress DNS::GoogleResolve(PCCHAR host, RequestType dnstype)
{
    auto serverName = "dns.google"_embed;
    IPAddress ip = ResolveOverHttp(host, IPAddress::FromIPv4(0x08080808), (PCCHAR)serverName, dnstype);
    if (!ip.IsValid())
        ip = ResolveOverHttp(host, IPAddress::FromIPv4(0x04040808), (PCCHAR)serverName, dnstype);
    return ip;
}

IPAddress DNS::Resolve(PCCHAR host, RequestType dnstype)
{
    LOG_DEBUG("Resolve(host: %s) called", host);

    IPAddress ip = CloudflareResolve(host, dnstype);
    if (ip.IsValid())
        return ip;

    ip = GoogleResolve(host, dnstype);
    if (ip.IsValid())
        return ip;

    if (dnstype == AAAA)
    {
        LOG_DEBUG("IPv6 resolution failed, falling back to IPv4 (A) for %s", host);
        ip = CloudflareResolve(host, A);
        if (ip.IsValid())
            return ip;

        return GoogleResolve(host, A);
    }

    return ip;
}
