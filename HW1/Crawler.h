#pragma once
class Crawler
{
public:
	CRITICAL_SECTION queueMutex;
	queue<string> Q;

	CRITICAL_SECTION hostMutex;
	unordered_set<string> seenHosts;

	CRITICAL_SECTION ipMutex;
	unordered_set<ULONG> seenIPs;

	// statistical variables
	LONG volatile activeThreadsCnt;
	LONG volatile extractedUrlsCnt;
	LONG volatile uqHostUrlsCnt;
	LONG volatile succDnsCnt;
	LONG volatile uqIpUrlsCnt;
	LONG volatile succRobotCnt;
	LONG volatile succCrawledUrlCnt;
	LONG volatile totalLinks;

	LONG volatile totalPages;
	LONG volatile totalBytes;

	LONG volatile httpCode2xx;
	LONG volatile httpCode3xx;
	LONG volatile httpCode4xx;
	LONG volatile httpCode5xx;
	LONG volatile httpCodeOther;

	HANDLE eventQuit;

	// method
	Crawler();
	~Crawler();

	void Crawl(HTMLParserBase* htmlParser, string url);
	void Stats();


};

