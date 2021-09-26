#pragma once
#include "Crawler.h"

class Request: public Crawler
{
public:
	URLParser* urlParser;
	HTMLParserBase* htmlParser;
	Socket* sock;
	in_addr hostAddr;

	Request(HTMLParserBase* htmlParser);
	~Request();

	// TODO: request return stats info
	void RequestURL(string url);
	bool DnsLookup(string host);
};

