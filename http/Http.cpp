#include "Http.h"
#include "Util.h"

HTTPRequest::HTTPRequest(string method, string url, string body)
{
    this->method = method;
    this->url = url;
    this->body = body;
	CURL *curl;
	CURLcode res;
	this->res = res;
	this->curl = curl_easy_init();
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

string HTTPRequest::GetToken(string refreshToken, string dormitory)
{
	string readBuffer;
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  	curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
	struct curl_slist *headres = NULL;
	string cookie = "Cookie: RefreshToken="+refreshToken;
	headres = curl_slist_append(headres, cookie.c_str());
	headres = curl_slist_append(headres, ("X-DormitoryId: "+dormitory).c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headres);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	const char *json = readBuffer.c_str();
	Json::Value payloadJson;
	Json::Reader r;
	r.parse(readBuffer, payloadJson);
	if (payloadJson.isObject() && payloadJson.isMember("token"))
	{
		string access_token = payloadJson["token"].asString();
		return access_token;
	}
	return readBuffer;
}

string HTTPRequest::UploadFile(string refreshToken, string dormitory, string pathFile)
{
	string readBuffer;
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  	curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
	struct curl_slist *headres = NULL;
	string cookie = "Cookie: Token="+GetToken(refreshToken, dormitory);
	headres = curl_slist_append(headres, cookie.c_str());
	headres = curl_slist_append(headres, ("X-DormitoryId: "+dormitory).c_str());
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
	return readBuffer;
}

