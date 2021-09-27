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
		EnterCriticalSection(&(crawler->queueMutex)); // lock mutex
		if (crawler->Q.size() == 0)
		{
			LeaveCriticalSection(&(crawler->queueMutex));
			break;
		}

		string url = crawler->Q.front(); crawler->Q.pop();
		LeaveCriticalSection(&(crawler->queueMutex));

		// stats purpose
		InterlockedIncrement(&crawler->extractedUrlsCnt);

		crawler->Crawl(htmlParser, url);
	}

	InterlockedDecrement(&crawler->activeThreadsCnt);
	delete htmlParser;
	return 0;
}

DWORD WINAPI threadStats(LPVOID pParam) {
	Crawler* crawler = ((Crawler*)pParam);
	crawler->Stats();
	return 0;
}

int runCrawler(Crawler* crawler, int threadNum) {
	HANDLE* handles = new HANDLE[threadNum];
	HANDLE statsHandle;

	statsHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadStats, crawler, 0, NULL);
	if (statsHandle == NULL) {
		printf("failed to create stats thread\n");
		return 1;
	}

	for (int i = 0; i < threadNum; ++i)
	{
		handles[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadCrawler, crawler, 0, NULL);
		if (handles[i] == NULL) {
			printf("failed to create crawler thread %d\n", i);
			return 1;
		}

		InterlockedIncrement(&crawler->activeThreadsCnt);
	}

	for (int i = 0; i < threadNum; ++i)
	{
		WaitForSingleObject(handles[i], INFINITE);
		CloseHandle(handles[i]);
	}

	SetEvent(crawler->eventQuit);
	WaitForSingleObject(statsHandle, 5000);
	CloseHandle(statsHandle);

	return 0;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("invalid argument.\n");
		printf("[Usage] HW1.exe $threadNum $file\n");
		exit(1);
	}

	int threadNum = strtol(argv[1], NULL, 10);
	if (threadNum <= 0) {
		printf("invalid number of threads\n");
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
