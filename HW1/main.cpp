// HW1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#pragma comment(lib, "ws2_32.lib")

void winsock_test(void)
{
	char host[] = "irl.cse.tamu.edu";
	u_short port = 80;
	char path[] = "/contact/";

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
	server.sin_port = htons(port);		// host-to-network flips the byte order

	// connect to the server on port 80
	if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		printf("Connection error: %d\n", WSAGetLastError());
		return;
	}

	printf("Successfully connected to %s (%s) on port %d\n", host, inet_ntoa(server.sin_addr), htons(server.sin_port));

	// send HTTP requests here

	const char *requestFmt = "GET %s HTTP/1.0\r\nUser-agent: myTAMUcrawler/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n";
	char sendBuf[4096];
	snprintf(sendBuf, sizeof(sendBuf), requestFmt, path, host);

	printf("len: %d str:%s", sizeof(sendBuf), sendBuf);
	if (send(sock, sendBuf, sizeof(sendBuf), 0) == SOCKET_ERROR) {
		printf("Send error: %d\n", WSAGetLastError);
		return;
	}

	printf("byte send\n");

	// read
	int initialBufSize = 50;
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
				int newSize = allocatedSize + 2048;
				readBuf = (char*)realloc(readBuf, newSize);
				allocatedSize = newSize;
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


	int status = 0;
	char* startOfStatus = strstr(readBuf, "HTTP/");
	if (startOfStatus != NULL)
	{
		char skipStr[] = "HTTP/1.X ";
		startOfStatus += strlen(skipStr);		
		char statusStr[] = "0000";
		strncpy_s(statusStr,strlen(statusStr), startOfStatus, 3);

		status = atoi(statusStr);
	}

	printf("status code: %d\n", status);

	// create new parser object
	HTMLParserBase* parser = new HTMLParserBase;

	char baseUrl[] = "http://www.tamu.edu";		// where this page came from; needed for construction of relative links

	int nLinks;
	char* linkBuffer = parser->Parse(readBuf, curPos, baseUrl, (int)strlen(baseUrl), &nLinks);

	// check for errors indicated by negative values
	if (nLinks < 0)
		nLinks = 0;

	printf("Found %d links:\n", nLinks);



}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("invalid argument.\n");
		printf("[Usage] HW1.exe $url\n");
		exit(0);
	}

	printf("URL: %s\n", argv[1]);

	printf("\tParsing URL... ");
	URLParser *urlParser = new URLParser();
	bool ret = urlParser->parse(argv[1]);
	if (!ret) {
		exit(0);
	}

	printf("host %s, port %d, request %s\n", 
		urlParser->host.c_str(), 
		urlParser->port, 
		urlParser->getRequest().c_str());

	Socket* sock = new Socket();
	ret = sock->Send(urlParser);
	if (!ret) {
		exit(0);
	}

	clock_t timer = clock();
	printf("\tLoading... ");
	ret = sock->Read();
	if (!ret) {
		exit(0);
	}

	timer = clock() - timer;
	printf("done in %d ms with %d bytes\n", 1000 * timer/CLOCKS_PER_SEC, sock->curPos);

	//printf("%s\n", sock->buf);

	printf("\tVerifying header... ");
	int status = 0;
	char* startOfStatus = strstr(sock->buf, "HTTP/");
	if (startOfStatus != NULL)
	{
		char skipStr[] = "HTTP/1.X ";
		startOfStatus += strlen(skipStr);
		char statusStr[4];
		strncpy_s(statusStr, sizeof(statusStr), startOfStatus, 3);
		statusStr[3] = '\0';

		status = atoi(statusStr);
	}
	else {
		printf("failed with non-HTTP header\n");
		exit(0);
	}

	printf("status code %d\n", status);

	// create new parser object
	HTMLParserBase* parser = new HTMLParserBase();

	if (status >= 200 && status < 300) {
		char baseUrl[512];
		sprintf_s(baseUrl, "%s://%s", urlParser->scheme.c_str(), urlParser->host.c_str());

		timer = clock();
		printf("\t\b\b+ Parsing page... ");
		int nLinks;
		char* linkBuffer = parser->Parse(sock->buf, sock->curPos, baseUrl, (int)strlen(baseUrl), &nLinks);

		// check for errors indicated by negative values
		if (nLinks < 0)
			nLinks = 0;
		
		timer = clock() - timer;
		printf("done in %d ms with %d links\n", 1000* timer/ CLOCKS_PER_SEC, nLinks);
	}

	printf("--------------------------------------\n");


	char *endOfHeader = strstr(sock->buf, "\r\n\r\n");
	if (endOfHeader != nullptr) {
		int offset = endOfHeader - sock->buf;
		char* header = new char[offset + 1];
		strncpy_s(header, offset + 1, sock->buf, offset);
		printf("%s\n", header);
	}
	

	// printf("Found %d links:\n", nLinks);
	return 0;
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
