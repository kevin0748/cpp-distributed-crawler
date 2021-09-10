/**
	This is course project of CSCE612@2021Fall
	author: Chen Chuan Chang
*/

#include "pch.h"
#pragma comment(lib, "ws2_32.lib")

int parseResponseStatus(char *const buf) {
	printf("\tVerifying header... ");

	int status = 0;
	char* startOfStatus = strstr(buf, "HTTP/");
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
		exit(1);
	}

	return status;
}

void requestURL(Socket* sock, const char* url) {
	URLParser* urlParser = new URLParser();
	HTMLParserBase* parser = new HTMLParserBase();
	
	printf("URL: %s\n", url);

	printf("\tParsing URL... ");
	bool ret = urlParser->parse(url);
	if (!ret) {
		exit(1);
	}

	printf("host %s, port %d, request %s\n",
		urlParser->host.c_str(),
		urlParser->port,
		urlParser->getRequest().c_str());


	ret = sock->Send(urlParser, HTTP_GET);
	if (!ret) {
		exit(1);
	}

	clock_t timer = clock();
	printf("\tLoading... ");
	ret = sock->Read(MAX_PAGE_DOWNLOAD_SIZE);
	if (!ret) {
		exit(1);
	}

	timer = clock() - timer;
	printf("done in %d ms with %d bytes\n", 1000 * timer / CLOCKS_PER_SEC, sock->curPos);

	//printf("%s\n", sock->buf);

	int status = parseResponseStatus(sock->buf);
	printf("status code %d\n", status);

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
		printf("done in %d ms with %d links\n", 1000 * timer / CLOCKS_PER_SEC, nLinks);
	}

	printf("--------------------------------------\n");


	char* endOfHeader = strstr(sock->buf, "\r\n\r\n");
	if (endOfHeader != nullptr) {
		int offset = endOfHeader - sock->buf;
		string::size_type size = offset + 1;
		char *header = new char[size];
		strncpy_s(header, size, sock->buf, offset);
		printf("%s\n", header);
		delete []header;
	}

	delete parser;
	delete urlParser;

	return;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("invalid argument.\n");
		printf("[Usage] HW1.exe $url\n");
		exit(0);
	}

	Socket* sock = new Socket();

	requestURL(sock, argv[1]);
	
	delete sock;

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
