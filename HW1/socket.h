/**
    This is course project of CSCE612@2021Fall
    author: Chen Chuan Chang
*/

#pragma once

#define MAX_PAGE_DOWNLOAD_SIZE 2 * 1024 * 1024
#define MAX_ROBOTS_DOWNLOAD_SIZE 16 * 1024
#define MAX_DOWNLOAD_TIME 10 // in second

#define HTTP_GET "GET"
#define HTTP_HEAD "HEAD"

class Socket
{
public:
    SOCKET sock;       // socket handle
    char* buf;         // current buffer
    int allocatedSize; // bytes allocated for buf
    int curPos;        // current position in buffer

    Socket();
    ~Socket();
    bool Send(URLParser* urlParser, const char* method);
    bool Read(int maxDownloadSize);
};