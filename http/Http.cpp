#include "Http.h"
#include "Util.h"
#include "Log.h"

HTTPRequest::HTTPRequest()
{
}

HTTPRequest::~HTTPRequest()
{
}

void HTTPRequest::setMethod(string method)
{
	this->method = method;
}

void HTTPRequest::setUrl(string url)
{
	this->url = url;
}

void HTTPRequest::setToken(string token)
{
	this->token = token;
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
	((string *)userp)->append((char *)contents, size * nmemb);
	return size * nmemb;
}

string HTTPRequest::GetToken(string refreshToken, string dormitory)
{
	CURLcode res;
	CURL *curl_getToken = curl_easy_init();
	string readBuffer = "";
	string access_token = "";
	if (curl_getToken)
	{
		curl_easy_setopt(curl_getToken, CURLOPT_CUSTOMREQUEST, method.c_str());
		curl_easy_setopt(curl_getToken, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl_getToken, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl_getToken, CURLOPT_DEFAULT_PROTOCOL, "https");
		struct curl_slist *headers = NULL;
		string dormitory_headers = "X-DormitoryId: " + dormitory;
		string cookie_headers = "Cookie: RefreshToken=" + refreshToken;
		headers = curl_slist_append(headers, dormitory_headers.c_str());
		headers = curl_slist_append(headers, cookie_headers.c_str());
		curl_easy_setopt(curl_getToken, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl_getToken, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl_getToken, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl_getToken);
		if (res != CURLE_OK)
		{
			LOGW("get token error");
		}
		curl_easy_cleanup(curl_getToken);
		curl_getToken = NULL;
	}
	Json::Value payloadJson;
	if (payloadJson.parse(readBuffer) && payloadJson.isObject() && payloadJson.isMember("token"))
	{
		access_token = payloadJson["token"].asString();
		LOGD("Access token %s", access_token.c_str());
		return access_token;
	}
	return readBuffer;
}

string HTTPRequest::UploadFile(string refreshToken, string dormitory, string pathFile)
{
	string readBuffer = "";
	CURLcode res;
	CURL *curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
		struct curl_slist *headres = NULL;
		string cookie_headers = "Cookie: Token=" + token;
		string dormitory_headers = "X-DormitoryId: " + dormitory;
		LOGE("method: %s", method.c_str());
		LOGE("url: %s", url.c_str());
		LOGE("cookie_headers: %s", cookie_headers.c_str());
		LOGE("dormitory_headers: %s", dormitory_headers.c_str());
		headres = curl_slist_append(headres, dormitory_headers.c_str());
		headres = curl_slist_append(headres, cookie_headers.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headres);

		curl_mime *mime;
		curl_mimepart *part;
		mime = curl_mime_init(curl);
		part = curl_mime_addpart(mime);
		curl_mime_name(part, "file");
		LOGE("file: %s", pathFile.c_str());
		curl_mime_filedata(part, pathFile.c_str());
		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			LOGW("upload error");
		}
		curl_mime_free(mime);
		curl_easy_cleanup(curl);
		curl = NULL;
	}
	return readBuffer;
}

bool HTTPRequest::CreateBackup(string refreshToken, string dormitory, string mac, string version, string size, string path, string hcId)
{
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, ("X-DormitoryId: " + dormitory).c_str());
		headers = curl_slist_append(headers, ("Cookie: Token=" + token).c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		Json::Value dataJson;
		dataJson["name"] = "hc-" + mac;
		dataJson["version"] = version;
		dataJson["size"] = size;
		dataJson["url"] = path;
		dataJson["homeControllerId"] = hcId;
		// const char *data = "{\n  \"name\": \"HC-C283\",\n  \"version\": \"1.2.9\",\n  \"size\": \"1.01 MB\",\n  \"url\": \"/home-controller/12038/133148503667486921/rd.Sqlite\",\n  \"homeControllerId\": \"5ab14c1b-6571-503f-a6d0-3157ec44e095\"\n}";
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataJson.toString().c_str());
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			LOGW("Create backup Error");
			return false;
		}
		curl_easy_cleanup(curl);
		curl = NULL;
		return true;
	}
	return false;
}

string HTTPRequest::DownloadFile(string dormitory)
{
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	string readBuffer = "";
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
		struct curl_slist *headers = NULL;
		string cookie_headers = "Cookie: Token=" + token;
		string dormitory_headers = "X-DormitoryId: " + dormitory;
		headers = curl_slist_append(headers, dormitory_headers.c_str());
		headers = curl_slist_append(headers, cookie_headers.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			LOGW("get token error");
		}
		curl_easy_cleanup(curl);
		curl = NULL;
	}
	return readBuffer;
}

string HTTPRequest::GetWeather(float longitude, float latitude)
{
	return GetWeather(to_string(latitude), to_string(longitude));
}

string HTTPRequest::GetWeather(string longitude, string latitude)
{
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	string readBuffer = "";
	if (curl)
	{
		LOGW("method %s", method.c_str());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https://api.openweathermap.org/data/2.5/weather?lat=" + latitude + "&lon=" + longitude + "&appid=ebd13e00acf60358e311499f1701ffc2&units=metric");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");

		struct curl_slist *headers = NULL;
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			LOGW("get token error");
		}
		curl_easy_cleanup(curl);
		curl = NULL;
	}
	return readBuffer;
}