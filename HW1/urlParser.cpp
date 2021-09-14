/**
	This is course project of CSCE612@2021Fall
	author: Chen Chuan Chang
*/

#include "pch.h"

unordered_set<ULONG> seenIPs;

URLParser::URLParser() {
	hostAddr.S_un.S_addr = 0;
}

URLParser::~URLParser() {
}

bool URLParser::parse(const char* url)
{
	string sUrl(url);

	size_t schemeEnd = sUrl.find("://");
	if (schemeEnd == string::npos)
	{
		printf("url parse failed: missing scheme\n");
		return false;
	}
	else
	{
		scheme = sUrl.substr(0, schemeEnd);
		sUrl = sUrl.substr(schemeEnd + 3); // skip `://`
	}

	if (scheme != "http") {
		printf("failed with invalid scheme\n");
		return false;
	}

	size_t fragmentStart = sUrl.find('#');
	if (fragmentStart != string::npos)
	{
		fragment = sUrl.substr(fragmentStart);
		sUrl = sUrl.substr(0, fragmentStart);
	}

	size_t queryStart = sUrl.find('?');
	if (queryStart != string::npos)
	{
		query = sUrl.substr(queryStart + 1, fragmentStart);
		sUrl = sUrl.substr(0, queryStart);
	}

	size_t hostPortEnd = sUrl.find("/");
	if (hostPortEnd != string::npos)
	{
		string hostPort = sUrl.substr(0, hostPortEnd);
		sUrl = sUrl.substr(hostPortEnd);
		path = sUrl;

		if (!parseHostPort(hostPort))
		{
			return false;
		}
	}
	else // no path
	{
		path = "/";

		if (!parseHostPort(sUrl))
		{
			return false;
		}
	}

	//print();
}

bool URLParser::parseHostPort(string hostPort)
{
	size_t portStart = hostPort.find(":");
	if (portStart != string::npos)
	{
		if (portStart + 1 >= hostPort.size()) {
			printf("failed with invalid port\n");
			return false;
		}

		port = stoi(hostPort.substr(portStart + 1));
		host = hostPort.substr(0, portStart);
	}
	else
	{
		port = defaultPort();
		host = hostPort;
	}

	if (host.size() == 0)
	{
		printf("url parse failed: missing host\n");
		return false;
	}
	else if (host.size() >= MAX_HOST_LEN)
	{
		printf("failed with host len larger than MAX_HOST_LEN\n");
	}

	if (port == 0) {
		printf("failed with invalid port\n");
		return false;
	}

	return true;
}

int URLParser::defaultPort()
{
	if (scheme == "http")
	{
		return 80;
	}
	/*else if (scheme == "https")
	{
		return 443;
	}*/

	return 0;
}

void URLParser::print()
{
	cout << "scheme: " << scheme << endl;
	cout << "host: " << host << endl;
	cout << "port: " << port << endl;
	cout << "path: " << path << endl;
	cout << "fragment: " << fragment << endl;
	cout << "query: " << query << endl;
}

string URLParser::getRequest() {
	string request = path;
	if (query.size() > 0) {
		request += "?" + query;
	}

	return request;
}

string URLParser::getRobots() {
	return "/robots.txt";
}

bool URLParser::dnsLookup() {
	if (hostAddr.S_un.S_addr != 0) {
		// dns lookup is done 
		return true;
	}

	// structure used in DNS lookups
	struct hostent* remote;

	clock_t timer = clock();
	printf("\tDoing DNS... ");
	// first assume that the string is an IP address
	const char* hostChars = host.c_str();
	DWORD IP = inet_addr(hostChars);
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		if ((remote = gethostbyname(hostChars)) == NULL)
		{
			printf("failed with %d\n", WSAGetLastError());
			//printf("Invalid string: neither FQDN, nor IP address\n");
			return false;
		}
		else // take the first IP address and copy into sin_addr
			memcpy((char*)&(hostAddr), remote->h_addr, remote->h_length);
	}
	else
	{
		// if a valid IP, directly drop its binary version into sin_addr
		hostAddr.S_un.S_addr = IP;
	}

	timer = clock() - timer;
	printf("done in %d ms, found %s\n", 1000 * timer / CLOCKS_PER_SEC, inet_ntoa(hostAddr));

	printf("\tChecking IP uniqueness... ");
	//printf("int: %d\n str: %s\n", hostAddr.S_un.S_addr, inet_ntoa(hostAddr));
	if (seenIPs.find(hostAddr.S_un.S_addr) != seenIPs.end()) {
		printf("failed\n");
		return false;
	}

	seenIPs.insert(hostAddr.S_un.S_addr);
	printf("passed\n");

	return true;
}