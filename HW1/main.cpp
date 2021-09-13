/**
	This is course project of CSCE612@2021Fall
	author: Chen Chuan Chang
*/

#include "pch.h"
#pragma comment(lib, "ws2_32.lib")

int parseResponseStatus(char* const buf) {
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
		return -1;
	}

	printf("status code %d\n", status);
	return status;
}

void requestURL(const char* url) {
	URLParser* urlParser = new URLParser();
	HTMLParserBase* htmlParser = new HTMLParserBase();
	Socket* sock = new Socket();

	printf("URL: %s\n", url);

	printf("\tParsing URL... ");
	bool ret = urlParser->parse(url);
	if (!ret) {
		return;
	}

	printf("host %s, port %d, request %s\n",
		urlParser->host.c_str(),
		urlParser->port,
		urlParser->getRequest().c_str());


	// check host uniqueness

	// robot
	ret = sock->Send(urlParser, HTTP_HEAD);
	if (!ret) {
		return;
	}

	clock_t timer = clock();
	printf("\tLoading... ");
	ret = sock->Read(MAX_ROBOTS_DOWNLOAD_SIZE);
	if (!ret) {
		return;
	}

	timer = clock() - timer;
	printf("done in %d ms with %d bytes\n", 1000 * timer / CLOCKS_PER_SEC, sock->curPos);

	//printf("%s\n", sock->buf);

	int status = parseResponseStatus(sock->buf);
	if (status < 400 || status >= 500) {
		// robots.txt exists. We should skip this website.
		return;
	}

	delete sock;

	sock = new Socket();

	ret = sock->Send(urlParser, HTTP_GET);
	if (!ret) {
		return;
	}

	printf("\tLoading... ");
	ret = sock->Read(MAX_PAGE_DOWNLOAD_SIZE);
	if (!ret) {
		return;
	}

	timer = clock() - timer;
	printf("done in %d ms with %d bytes\n", 1000 * timer / CLOCKS_PER_SEC, sock->curPos);

	//printf("%s\n", sock->buf);

	status = parseResponseStatus(sock->buf);
	if (status == -1) {
		return;
	}
	else if (status >= 200 && status < 300) {
		char baseUrl[512];
		sprintf_s(baseUrl, "%s://%s", urlParser->scheme.c_str(), urlParser->host.c_str());

		timer = clock();
		printf("\t\b\b+ Parsing page... ");
		int nLinks;
		char* linkBuffer = htmlParser->Parse(sock->buf, sock->curPos, baseUrl, (int)strlen(baseUrl), &nLinks);

		// check for errors indicated by negative values
		if (nLinks < 0)
			nLinks = 0;

		timer = clock() - timer;
		printf("done in %d ms with %d links\n", 1000 * timer / CLOCKS_PER_SEC, nLinks);
	}

	delete htmlParser;
	delete urlParser;
	delete sock;

	return;
}

void parseAndRequestURLs(const char* fileBuf, int fileSize) {
	int readCursor = 0;
	while (readCursor < fileSize) {
		int lineLen = 0;
		const char *endOfLine = strchr(fileBuf, '\n');
		if (endOfLine == NULL) {
			lineLen = strlen(fileBuf);
		}
		else {
			lineLen = endOfLine - fileBuf + 1;
		}

		char *line = new char[lineLen + 1];
		strncpy_s(line, lineLen + 1, fileBuf, lineLen);
		line[lineLen] = NULL;
		//printf("%d: %s\n", i++, url);

		int urlLen = lineLen;
		if (strchr(line, '\n')) {
			--urlLen;
		}
		if (strchr(line, '\r')) {
			--urlLen;
		}
		char* url = new char[urlLen + 1];
		strncpy_s(url, urlLen + 1, line, urlLen);
		url[urlLen] = NULL;

		requestURL(url);
		
		fileBuf += lineLen;
		readCursor += lineLen;

		delete[]line;
		delete[]url;
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("invalid argument.\n");
		printf("[Usage] HW1.exe $url\n");
		exit(0);
	}

	char filename[] = "URL-input-100.txt";

	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with %d\n", GetLastError());
		return 0;
	}

	// get file size
	LARGE_INTEGER li;
	BOOL bRet = GetFileSizeEx(hFile, &li);
	// process errors
	if (bRet == 0)
	{
		printf("GetFileSizeEx error %d\n", GetLastError());
		return 0;
	}

	// read file into a buffer
	int fileSize = (DWORD)li.QuadPart;			// assumes file size is below 2GB; otherwise, an __int64 is needed
	DWORD bytesRead;
	// allocate buffer
	char* fileBuf = new char[fileSize + 1];
	// read into the buffer
	bRet = ReadFile(hFile, fileBuf, fileSize, &bytesRead, NULL);
	// process errors
	if (bRet == 0 || bytesRead != fileSize)
	{
		printf("ReadFile failed with %d\n", GetLastError());
		return 0;
	}
	fileBuf[fileSize] = NULL;

	parseAndRequestURLs(fileBuf, fileSize);

	// done with the file
	CloseHandle(hFile);
	delete fileBuf;

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
