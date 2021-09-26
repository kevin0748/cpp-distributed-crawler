#include "pch.h"

Crawler::Crawler() {
	InitializeCriticalSection(&crawlerMutex);
	InitializeCriticalSection(&hostMutex);
	InitializeCriticalSection(&ipMutex);
}

Crawler::~Crawler() {
	DeleteCriticalSection(&crawlerMutex);
	DeleteCriticalSection(&hostMutex);
	DeleteCriticalSection(&ipMutex);
}

void Crawler::Crawl(HTMLParserBase *htmlParser, string url) {
	Request* req = new Request(htmlParser);
	req->RequestURL(url);
	delete req;
}

void Crawler::Stats() {

}