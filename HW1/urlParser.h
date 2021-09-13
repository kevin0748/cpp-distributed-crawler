/**
    This is course project of CSCE612@2021Fall
    author: Chen Chuan Chang
*/

#pragma once

#include <string>
#include <windows.h>


using namespace std;

class URLParser
{
public:
    string scheme;
    string host;
    int port = 0;
    string path;
    string query;
    string fragment;

    bool parse(const char* url);
   
    bool parseHostPort(string hostPort);
    
    int defaultPort();
    
    void print();   

    string getRequest();

    string getRobots();
};