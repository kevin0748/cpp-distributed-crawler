#pragma once
class Crawler
{
public:
	queue<string> Q;

	CRITICAL_SECTION hostMutex;
	unordered_set<string> seenHosts;

	CRITICAL_SECTION ipMutex;
	unordered_set<ULONG> seenIPs;

	// statistical variables
	int activeThreadsCnt;
	int extractedUrlsCnt;
	int uniqueHostUrlsCnt;
	int successDnsCnt;
	int uniqueIpUrlsCnt;
	int passRobotCnt;
	int successCrawledUrlCnt;
	int totalLink;

	int totalPages;
	int totalBytes;

	unordered_map<string,int> httpCodes;

	// multi-thread 
	CRITICAL_SECTION crawlerMutex;
	HANDLE eventQuit;

	// method
	Crawler();
	~Crawler();

	void Crawl(HTMLParserBase* htmlParser, string url);
	void Stats();


};

