/**
    This is course project of CSCE612@2021Fall
    author: Chen Chuan Chang
*/

#include "pch.h"

#define INITIAL_BUF_SIZE 4096
#define THRESHOLD 1024


Socket::Socket()
{
    WSADATA wsaData;

    //Initialize WinSock; once per program run
    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        printf("WSAStartup error %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    // open a TCP socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        printf("socket() generated error %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    // create this buffer once, then possibly reuse for multiple connections in Part 3
    buf = (char*)malloc(INITIAL_BUF_SIZE);
    allocatedSize = INITIAL_BUF_SIZE;
}

Socket::~Socket() {
    delete buf;

    // close the socket to this server; open again for the next one
    closesocket(sock);

    // call cleanup when done with everything and ready to exit program
    WSACleanup();
}

bool Socket::Send(URLParser* urlParser, const char* method) {
    // structure used in DNS lookups
    struct hostent* remote;

    // structure for connecting to server
    struct sockaddr_in server;

    clock_t timer = clock();
    printf("\tDoing DNS... ");
    // first assume that the string is an IP address
    const char* host = urlParser->host.c_str();
    DWORD IP = inet_addr(host);
    if (IP == INADDR_NONE)
    {
        // if not a valid IP, then do a DNS lookup
        if ((remote = gethostbyname(host)) == NULL)
        {
            printf("failed with %d", WSAGetLastError());
            //printf("Invalid string: neither FQDN, nor IP address\n");
            return false;
        }
        else // take the first IP address and copy into sin_addr
            memcpy((char*)&(server.sin_addr), remote->h_addr, remote->h_length);
    }
    else
    {
        // if a valid IP, directly drop its binary version into sin_addr
        server.sin_addr.S_un.S_addr = IP;
    }

    timer = clock() - timer;
    printf("done in %d ms, found %s\n", 1000*timer/CLOCKS_PER_SEC, inet_ntoa(server.sin_addr));

    // setup the port # and protocol type
    server.sin_family = AF_INET;
    server.sin_port = htons(urlParser->port);		// host-to-network flips the byte order

    timer = clock();
    printf("\t\b\b* Connecting on page... ");
    // connect to the server on port 80
    if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
    {
        printf("Connection error: %d\n", WSAGetLastError());
        return false;
    }
    timer = clock() - timer;
    printf("done in %d ms\n", 1000 * timer / CLOCKS_PER_SEC);

    // printf("Successfully connected to %s (%s) on port %d\n", host, inet_ntoa(server.sin_addr), htons(server.sin_port));

    const char* requestFmt = "%s %s HTTP/1.0\r\nUser-agent: myTAMUcrawler/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n";
    char sendBuf[MAX_REQUEST_LEN];
    snprintf(sendBuf, sizeof(sendBuf), requestFmt, method, urlParser->getRequest().c_str(), urlParser->host.c_str());

    printf("len: %d str:%s", sizeof(sendBuf), sendBuf);
    if (send(sock, sendBuf, sizeof(sendBuf), 0) == SOCKET_ERROR) {
        printf("Send error: %d\n", WSAGetLastError);
        return false;
    }

    // printf("byte send\n");
    return true;
}

bool Socket::Read(int maxDownloadSize)
{
    fd_set rfd;
 
    int ret;
    int threshold = 10;

    // set timeout to 10 seconds
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    while (true)
    {
        FD_ZERO(&rfd);
        FD_SET(sock, &rfd);

        // wait to see if socket has any data (see MSDN)
        if ((ret = select(0, &rfd, nullptr, nullptr, &timeout)) > 0)
        {
            // new data available; now read the next segment

            int bytes = recv(sock, buf + curPos, allocatedSize - curPos, 0);
            if (bytes == SOCKET_ERROR) {
                printf("failed with %d on recv\n", WSAGetLastError());
                break;
            }

            if (bytes == 0) {
                // printf("conection close\n");
                buf[curPos] = '\0';
                return true; // normal completion
            }

            curPos += bytes; // adjust where the next recv goes
            if (allocatedSize - curPos < THRESHOLD) {
                int newSize = allocatedSize * 2;
                char *newbuf = (char*)realloc(buf, newSize);
                if (newbuf == nullptr) {
                    printf("failed to realloc recv buffer\n");
                    return false;
                }
                buf = newbuf;
                allocatedSize = newSize;
            }
            // resize buffer; you can use realloc(), HeapReAlloc(), or
            // memcpy the buffer into a bigger array
        }
        else if (ret == 0) {
            // report timeout
            printf("recv: timeout\n");
            break;
        }
        else {
            printf("recv error: %d\n", WSAGetLastError());
            break;
        }
    }
    return false;
}