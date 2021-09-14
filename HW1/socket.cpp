/**
    This is course project of CSCE612@2021Fall
    author: Chen Chuan Chang
*/

#include "pch.h"


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

bool Socket::Send(URLParser* urlParser, SendType sendType) {
    if (sendType != robots && sendType != page) {
        printf("socket send: internal error\n");
        return false;
    }

    if (!urlParser->dnsLookup()) {
        return false;
    }

    // structure for connecting to server
    struct sockaddr_in server;
    server.sin_addr = urlParser->hostAddr;

    // setup the port # and protocol type
    server.sin_family = AF_INET;
    server.sin_port = htons(urlParser->port);		// host-to-network flips the byte order

    clock_t timer = clock();
    if (sendType == robots) {
        printf("\tConnecting on robots... ");
    }
    else if (sendType == page) {
        printf("\t\b\b* Connecting on page... ");
    }
    
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

    if ( sendType == robots) {
        snprintf(sendBuf, sizeof(sendBuf), requestFmt, HTTP_HEAD, "/robots.txt", urlParser->host.c_str());
    } else if (sendType == page) {
        snprintf(sendBuf, sizeof(sendBuf), requestFmt, HTTP_GET, urlParser->getRequest().c_str(), urlParser->host.c_str());
    }
    
    // printf("len: %d str:%s\n", sizeof(sendBuf), sendBuf);
    if (send(sock, sendBuf, sizeof(sendBuf), 0) == SOCKET_ERROR) {
        printf("Send error: %d\n", WSAGetLastError);
        return false;
    }

    // printf("byte send\n");
    return true;
}

bool makeRequest(const char* method, char* request) {
    if (strcmp(method, HTTP_HEAD) == 0) {

        return true;
    } else if (strcmp(method, HTTP_GET) == 0) {
        return true;
    }
    
    return false;
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

    clock_t downloadTimer = clock();

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
                buf[curPos] = NULL;
                return true; // normal completion
            }

            curPos += bytes; // adjust where the next recv goes

            if (curPos > maxDownloadSize) {
                printf("exceed maxDownloadSize\n");
                return false;
            }
            if ( (clock() - downloadTimer)/CLOCKS_PER_SEC > MAX_DOWNLOAD_TIME) {
                printf("exceed maxDownloadTime\n");
                return false;
            }

            // resize buffer; you can use realloc(), HeapReAlloc(), or
            // memcpy the buffer into a bigger array
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
