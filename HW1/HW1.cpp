// HW1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#pragma comment(lib, "ws2_32.lib")

void winsock_test(void)
{
	char host[] = "www.yahoo.com";
	char path[] = "/";
	// string pointing to an HTTP server (DNS name or IP)
	// char str [] = "irl.cse.tamu.edu";
	//char str [] = "128.194.135.72";

	WSADATA wsaData;

	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	// open a TCP socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf("socket() generated error %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	// structure used in DNS lookups
	struct hostent* remote;

	// structure for connecting to server
	struct sockaddr_in server;

	// first assume that the string is an IP address
	DWORD IP = inet_addr(host);
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		if ((remote = gethostbyname(host)) == NULL)
		{
			printf("Invalid string: neither FQDN, nor IP address\n");
			return;
		}
		else // take the first IP address and copy into sin_addr
			memcpy((char*)&(server.sin_addr), remote->h_addr, remote->h_length);
	}
	else
	{
		// if a valid IP, directly drop its binary version into sin_addr
		server.sin_addr.S_un.S_addr = IP;
	}

	// setup the port # and protocol type
	server.sin_family = AF_INET;
	server.sin_port = htons(80);		// host-to-network flips the byte order

	// connect to the server on port 80
	if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		printf("Connection error: %d\n", WSAGetLastError());
		return;
	}

	printf("Successfully connected to %s (%s) on port %d\n", host, inet_ntoa(server.sin_addr), htons(server.sin_port));

	// send HTTP requests here

	char requestFmt[] = "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n";
	char sendBuf[1024];
	snprintf(sendBuf, 1024 - 1, requestFmt, path, host);

	//char sendBuf[] = "GET / HTTP/1.0\r\nHost: irl.cse.tamu.edu\r\nConnection: close\r\n\r\n";

	printf("len: %d str:%s", sizeof(sendBuf), sendBuf);
	if (send(sock, sendBuf, sizeof(sendBuf), 0) == SOCKET_ERROR) {
		printf("Send error: %d\n", WSAGetLastError);
		return;
	}

	printf("byte send\n");

	// read
	int initialBufSize = 10000;
	char* readBuf = new char[initialBufSize];
	int allocatedSize = initialBufSize;
	int curPos = 0;
	fd_set rfd;
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	int ret;
	int threshold = 10;

	while (true) {
		FD_ZERO(&rfd);
		FD_SET(sock, &rfd);
		if (ret = select(0, &rfd, nullptr, nullptr, &timeout) > 0) {
			int bytes = recv(sock, readBuf + curPos, allocatedSize - curPos, 0);
			if (bytes == SOCKET_ERROR) {
				printf("recv error: %d", bytes);
				return;
			}

			if (bytes == 0) {
				printf("conection close\n");
				break;
			}

			curPos += bytes;

			if (allocatedSize - curPos < threshold) {
				readBuf = (char*)realloc(readBuf, allocatedSize);
			}
		}
		else if (ret == 0) {
			printf("timeout");
			break;
		}
		else {
			printf("error: %d", WSAGetLastError());
			break;
		}
	}

	readBuf[curPos] = '\0';
	printf("%s\n", readBuf);


	char* status = strstr(readBuf, "HTTP/1.0");
	printf("status: %s\n", status);

	// close the socket to this server; open again for the next one
	closesocket(sock);

	// call cleanup when done with everything and ready to exit program
	WSACleanup();
}

int main() {
	winsock_test();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
