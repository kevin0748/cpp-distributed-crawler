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

#define INITIAL_BUF_SIZE 4096
#define THRESHOLD 1024

class Socket
{
public:
    enum SendType{ robots, page};

    SOCKET sock;       // socket handle
    char* buf;         // current buffer
    int allocatedSize; // bytes allocated for buf
    int curPos;        // current position in buffer

    Socket();
    ~Socket();
    bool Send(URLParser* urlParser, struct  in_addr hostAddr, SendType sendType);
    bool Read(int maxDownloadSize);
};