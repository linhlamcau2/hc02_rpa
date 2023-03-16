#pragma once

#include <string>
#include <map>
#include "json.h"
#include "File.h"

using namespace std;

class FileTransfer
{
private:
	string subFwTopic;
	string pubFwTopic;

	map<string, File *> files;

	void OnFWMessage(string &topic, char *payload, int payloadlen);

	int UploadChunk(string sessionId, File *file);

	int OnRpcUploadFileResp(Json::Value &reqValue, Json::Value &respValue);
	int OnRpcUploadBinaryResp(Json::Value &reqValue, Json::Value &respValue);

	int OnRpcDownloadFileResp(Json::Value &reqValue, Json::Value &respValue);

public:
	FileTransfer();

	void init();

	int uploadFile(string path, string name);
	int downloadFile(string path, string name);
};

extern FileTransfer *fileTransfer;
