#include "pch.h"

Crawler::Crawler() {
	activeThreadsCnt = 0;
	extractedUrlsCnt = 0;
	uqHostUrlsCnt = 0;
	succDnsCnt = 0;
	uqIpUrlsCnt = 0;
	succRobotCnt = 0;
	succCrawledUrlCnt = 0;
	totalLinks = 0;
	totalPages = 0;
	totalBytes = 0;
	
	httpCode2xx = 0;
	httpCode3xx = 0;
	httpCode4xx = 0;
	httpCode5xx = 0;
	httpCodeOther = 0;

	InitializeCriticalSection(&queueMutex);
	InitializeCriticalSection(&hostMutex);
	InitializeCriticalSection(&ipMutex);

	eventQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
}

Crawler::~Crawler() {
	DeleteCriticalSection(&queueMutex);
	DeleteCriticalSection(&hostMutex);
	DeleteCriticalSection(&ipMutex);
}

void Crawler::Crawl(HTMLParserBase *htmlParser, string url) {
	Request* req = new Request(htmlParser, this);
	req->RequestURL(url);
	delete req;
}

void Crawler::Stats() {
	clock_t startAt = clock();
	clock_t lastReportAt = clock();
	float elapse;
	LONG lastPages = 0, lastBytes = 0;

	while (WaitForSingleObject(eventQuit, 2000) == WAIT_TIMEOUT) {
		EnterCriticalSection(&queueMutex);
		int queueSize = Q.size();
		LeaveCriticalSection(&queueMutex);

		printf("[%3d] %4d Q %6d E %7d H %6d D %6d I %5d R %5d C %5d L %4.fK\n", 
			(clock() - startAt) / CLOCKS_PER_SEC,
			activeThreadsCnt,
			queueSize, // Q
			extractedUrlsCnt, // E
			uqHostUrlsCnt, // H
			succDnsCnt, // D
			uqIpUrlsCnt, // I
			succRobotCnt, // R
			succCrawledUrlCnt, // C
			(float)totalLinks / 1000.0); // L

		elapse = (float)(clock() - lastReportAt) / (float)CLOCKS_PER_SEC;
		printf("   *** crawling %.1f pps @ %.1f Mbps\n", 
			(float)(totalPages-lastPages)/elapse,
			(float)(totalBytes-lastBytes)/elapse * 8 / 1000000);

		lastReportAt = clock();
		lastPages = totalPages;
		lastBytes = totalBytes;
	}

	elapse = (clock() - startAt) / CLOCKS_PER_SEC;

	printf("\n");
	printf("Extracted %d URLs @ %1.f/s\n", extractedUrlsCnt, (float)extractedUrlsCnt/elapse);
	printf("Looked up %d DNS names @ %1.f/s\n", uqHostUrlsCnt, (float)uqHostUrlsCnt/elapse);
	printf("Attempted %d robots @ %1.f/s\n", uqIpUrlsCnt, (float)uqIpUrlsCnt/elapse);
	printf("Crawled %d pages @ %1.f/s (%.2f MB)\n", succCrawledUrlCnt, (float)succCrawledUrlCnt/elapse, (float)totalBytes / 1000000.0);
	printf("Parsed %d links @ %1.f/s\n", totalLinks, (float)totalLinks/elapse);
	printf("HTTP codes: 2xx = %d, 3xx = %d, 4xx = %d, 5xx = %d, other = 0\n", 
		httpCode2xx, httpCode3xx, httpCode4xx, httpCode5xx, httpCodeOther);

	return;
}