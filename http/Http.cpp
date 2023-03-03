#include "Http.h"
#include "Util.h"
#include "Log.h"

HTTPRequest::HTTPRequest(string method, string url, string body)
{
	this->method = method;
	this->url = url;
	this->body = body;
	curl = NULL;
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
	((string *)userp)->append((char *)contents, size * nmemb);
	return size * nmemb;
}

string HTTPRequest::GetToken(string refreshToken, string dormitory)
{
	CURLcode res;
	curl = curl_easy_init();
	string readBuffer;
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
		struct curl_slist *headers = NULL;
		string dormitory_headers = "X-DormitoryId: " + dormitory;
		string cookie_headers = "Cookie: RefreshToken=" + refreshToken;
		LOGE("method: %s", method.c_str());
		LOGE("url: %s", url.c_str());
		LOGE("dormitory_headers: %s",dormitory_headers.c_str());
		LOGE("cookie_headers: %s", cookie_headers.c_str());
		headers = curl_slist_append(headers, dormitory_headers.c_str());
		headers = curl_slist_append(headers, cookie_headers.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		curl = NULL;
	}
	LOGD("readBuffer: %s", readBuffer.c_str());
	Json::Value payloadJson;
	Json::Reader r;
	r.parse(readBuffer, payloadJson);
	if (payloadJson.isObject() && payloadJson.isMember("token"))
	{
		string access_token = payloadJson["token"].asString();
		LOGD("Access token %s", access_token.c_str());
		return access_token;
	}
	return readBuffer;
}

string HTTPRequest::UploadFile(string refreshToken, string dormitory, string pathFile)
{
	string readBuffer = "";
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
		struct curl_slist *headres = NULL;
		string cookie = "Cookie: Token=" + GetToken(refreshToken, dormitory);
		headres = curl_slist_append(headres, cookie.c_str());
		headres = curl_slist_append(headres, ("X-DormitoryId: " + dormitory).c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headres);
		curl_mime *mime;
		curl_mimepart *part;
		mime = curl_mime_init(curl);
		part = curl_mime_addpart(mime);
		curl_mime_name(part, "");
		curl_mime_filedata(part, pathFile.c_str());
		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_mime_free(mime);
		curl_easy_cleanup(curl);
		curl = NULL;
	}
	return readBuffer;
}
