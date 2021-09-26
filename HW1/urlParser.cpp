/**
	This is course project of CSCE612@2021Fall
	author: Chen Chuan Chang
*/

#include "pch.h"

URLParser::URLParser() {
}

URLParser::~URLParser() {
}

bool URLParser::parse(string url)
{
	size_t schemeEnd = url.find("://");
	if (schemeEnd == string::npos)
	{
		printf("url parse failed: missing scheme\n");
		return false;
	}
	else
	{
		scheme = url.substr(0, schemeEnd);
		url = url.substr(schemeEnd + 3); // skip `://`
	}

	if (scheme != "http") {
		printf("failed with invalid scheme\n");
		return false;
	}

	size_t fragmentStart = url.find('#');
	if (fragmentStart != string::npos)
	{
		fragment = url.substr(fragmentStart);
		url = url.substr(0, fragmentStart);
	}

	size_t queryStart = url.find('?');
	if (queryStart != string::npos)
	{
		query = url.substr(queryStart + 1, fragmentStart);
		url = url.substr(0, queryStart);
	}

	size_t hostPortEnd = url.find("/");
	if (hostPortEnd != string::npos)
	{
		string hostPort = url.substr(0, hostPortEnd);
		url = url.substr(hostPortEnd);
		path = url;

		if (!parseHostPort(hostPort))
		{
			return false;
		}
	}
	else // no path
	{
		path = "/";

		if (!parseHostPort(url))
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
