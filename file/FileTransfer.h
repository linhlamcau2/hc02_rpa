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

	bool UploadChunk(string sessionId, File *file);

	int OnRPCUploadFileResp(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCUploadBinaryResp(Json::Value &reqValue, Json::Value &respValue);

	int OnRPCDownloadFileResp(Json::Value &reqValue, Json::Value &respValue);

public:
	FileTransfer();

	void init();

	bool uploadFile(string path, string name);
	bool downloadFile(string path, string name);
};

extern FileTransfer *fileTransfer;
