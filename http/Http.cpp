#include "Http.h"
#include "Util.h"

HTTPRequest::HTTPRequest(string method, string url, string header, string body)
{
    this->method = method;
    this->url = url;
    this->body = body;
    this->header = header;
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
	((string *)userp)->append((char *)contents, size * nmemb);
	return size * nmemb;
}

string HTTPRequest::HTTPExecute()
{
    curl_global_init(CURL_GLOBAL_ALL);

	CURL *easyhandle = curl_easy_init();
	string readBuffer;
	curl_easy_setopt(easyhandle, CURLOPT_CUSTOMREQUEST, method.c_str());
	curl_easy_setopt(easyhandle, CURLOPT_URL, url.c_str());
	struct curl_slist *HTTPHeaders = NULL;
	HTTPHeaders = curl_slist_append(HTTPHeaders, header.c_str());
	HTTPHeaders = curl_slist_append(HTTPHeaders, "Content-Type: application/json");
	curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, HTTPHeaders);
	curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, body.c_str());
	curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &readBuffer);

	curl_easy_perform(easyhandle);
	curl_easy_cleanup(easyhandle);
	curl_slist_free_all(HTTPHeaders);
	return readBuffer;
}

string HTTPRequest::GetToken(string refreshToken)
{
	string getTokenHeader = "Cookie: RefreshToken="+refreshToken+"";
	string getTokenBody = "";
	HTTPRequest *res = new HTTPRequest("POST", RENEW_TOKEN, getTokenHeader, getTokenBody);
    string rspGetToken = res->HTTPExecute();
	const char *json = rspGetToken.c_str();
	Json::Value payloadJson;
	Json::Reader r;
	r.parse(rspGetToken, payloadJson);
	if (payloadJson.isObject() && payloadJson.isMember("token"))
	{
		string access_token = payloadJson["token"].asString();
		return access_token;
	}
	return rspGetToken;
}

