#pragma once

#include <string>
#include "json.h"

#ifndef BIN_PACKAGE_SIZE
#define BIN_PACKAGE_SIZE 4096
#endif

using namespace std;

class File
{
private:
	volatile bool isBusy;
	string filePath;
	streamsize fileSize;
	volatile int chunkIndex;
	int chunkCount;

	string subFwTopic;
	string pubFwTopic;

	void OnFWMessage(string &topic, string &payload);

	bool UploadChunk();

	int OnRPCUploadFileResp(Json::Value &reqValue, Json::Value &respValue);
	int OnRPCDownloadFileResp(Json::Value &reqValue, Json::Value &respValue);

public:
	File();

	void init();

	bool uploadFile(string path, string name);
	bool downloadFile(string path, string name);
};

extern File *file;
