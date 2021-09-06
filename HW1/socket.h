#pragma once

class Socket
{
public:
    SOCKET sock;       // socket handle
    char* buf;         // current buffer
    int allocatedSize; // bytes allocated for buf
    int curPos;        // current position in buffer

    Socket();
    ~Socket();
    bool Send(URLParser* urlParser);
    bool Read(void);
};