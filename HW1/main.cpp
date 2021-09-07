// HW1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#pragma comment(lib, "ws2_32.lib")

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
