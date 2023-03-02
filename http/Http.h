#pragma once

#include <iostream>
#include <string>
#include <curl/curl.h>

#define BASE_URL_DEV        "https://iot-dev.truesight.asia"
#define BASE_URL_PRO        "https://rallismartv2.rangdong.com.vn"

#define RENEW_TOKEN         "/rpc/iot-ebe/account/renew-token"

using namespace std;

#define my_sizeof(type) ((char *)(&type + 1) - (char *)(&type))
#define HEADER_SIZE 6

class HTTPRequest
{
private:
    string method;
    string url;
    string body;
    string header;

public:
    HTTPRequest(string method, string url, string header, string body);
    string HTTPExecute();
    string GetToken(string refreshToken);
};


// - get token
// - header 
