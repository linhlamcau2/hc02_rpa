#pragma once

#include <iostream>
#include <string.h>
#include <curl/curl.h>

#define BASE_URL_DEV        "https://iot-dev.truesight.asia"
#define BASE_URL_PRO        "https://rallismartv2.rangdong.com.vn"

#define RENEW_TOKEN         "/rpc/iot-ebe/account/renew-token"
#define HC_BACKUP_FILE_URL  "/rpc/iot-ebe/home-controller/upload-file"

#define POST                "POST"

using namespace std;

#define my_sizeof(type) ((char *)(&type + 1) - (char *)(&type))
#define HEADER_SIZE 6

class HTTPRequest
{
private:
    string method;
    string url;
    string body;
    CURL *curl;
    CURLcode res;

public:
    HTTPRequest(string method, string url, string body);
    string GetToken(string refreshToken, string dormitory);
    string UploadFile(string refreshToken, string dormitory, string pathFile);
};

