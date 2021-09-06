#include "pch.h"

bool URLParser::parse(char* url)
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

    print();
}

bool URLParser::parseHostPort(string hostPort)
{
    size_t portStart = hostPort.find(":");
    if (portStart != string::npos)
    {
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

    return true;
}

int URLParser::defaultPort()
{
    if (scheme == "http")
    {
        return 80;
    }
    else if (scheme == "https")
    {
        return 443;
    }

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