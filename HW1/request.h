#pragma once
class Request
{
public:
	URLParser* urlParser;
	HTMLParserBase* htmlParser;
	Socket* sock;

	Request(HTMLParserBase* htmlParser);
	~Request();

	void RequestURL(const char* url);
};

