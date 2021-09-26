/**
	This is course project of CSCE612@2021Fall
	author: Chen Chuan Chang
*/

#include "pch.h"
#pragma comment(lib, "ws2_32.lib")

void parseAndPushToCrawler(const char* fileBuf, int fileSize, Crawler *crawler) {
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

		string sUrl(url);
		crawler->Q.push(sUrl);
		
		fileBuf += lineLen;
		readCursor += lineLen;

		delete[]line;
		delete[]url;
	}
}

DWORD WINAPI threadCrawler(LPVOID pParam) {
	Crawler* crawler = ((Crawler*)pParam);
	HTMLParserBase* htmlParser = new HTMLParserBase();

	while (1) {
		EnterCriticalSection(&(crawler->crawlerMutex)); // lock mutex

		if (crawler->Q.size() == 0)
		{
			LeaveCriticalSection(&(crawler->crawlerMutex));
			break;
		}

		string url = crawler->Q.front(); crawler->Q.pop();
		LeaveCriticalSection(&(crawler->crawlerMutex));

		crawler->Crawl(htmlParser, url);
	}

	delete htmlParser;
	return 0;
}

DWORD WINAPI threadStats(LPVOID pParam) {
	Crawler* crawler = (Crawler*)(pParam);
	crawler->Stats();
	return 0;
}

int runCrawler(Crawler* crawler, int threadNum) {
	HANDLE* handles = new HANDLE[threadNum];
	for (int i = 0; i < threadNum; ++i)
	{
		handles[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadCrawler, crawler, 0, NULL);
		if (handles[i] == NULL) {
			printf("failed to create thread %d\n", i);
		}
	}


	for (int i = 0; i < threadNum; ++i)
	{
		WaitForSingleObject(handles[i], INFINITE);
		CloseHandle(handles[i]);
	}

	return 0;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("invalid argument.\n");
		printf("[Usage] HW1.exe 1 $file\n");
		exit(1);
	}

	if (strcmp(argv[1], "1") != 0) {
		printf("number of threads must be equal to 1\n");
		exit(1);
	}

	char *filename = argv[2];

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

	printf("Opened %s with size %d\n", filename, fileSize);

	int threadNum = 1000;
	Crawler crawler;
	parseAndPushToCrawler(fileBuf, fileSize, &crawler);

	runCrawler(&crawler, threadNum);

	// done with the file
	CloseHandle(hFile);
	delete[] fileBuf;

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
