#pragma once
class Request
{
public:
	URLParser* urlParser;
	HTMLParserBase* htmlParser;
	Socket* sock;

	Request();
	~Request();

	void RequestURL(const char* url);
};

