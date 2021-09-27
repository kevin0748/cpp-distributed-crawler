#pragma once
#include "Crawler.h"

class Request
{
public:
	HTMLParserBase* htmlParser;
	Crawler* crawler;

	URLParser* urlParser;
	Socket* sock;
	in_addr hostAddr;

	Request(HTMLParserBase* htmlParser, Crawler *crawler);
	~Request();

	// TODO: request return stats info
	void RequestURL(string url);
	bool DnsLookup(string host);
};

