#include "File.h"
#include <fstream>
#include <unistd.h>
#include "Gateway.h"
#include "Log.h"

File *file = NULL;

File::File()
{
	LOGD("File");
	isBusy = false;
}

void File::init()
{
	LOGD("init");
	subFwTopic = "/v1/server/hc/" + gateway->getMac() + "/bin/+";
	pubFwTopic = "/v1/hc/" + gateway->getMac() + "/bin/";
	gateway->cloudAddActionCallback(bind(&File::OnFWMessage, this, placeholders::_1, placeholders::_2, placeholders::_3), subFwTopic);

	gateway->OnDeviceRPCCallbackRegister("UploadFileResp", bind(&File::OnRPCUploadFileResp, this, placeholders::_1, placeholders::_2));
	gateway->OnDeviceRPCCallbackRegister("UploadBinaryResp", bind(&File::OnRPCUploadBinaryResp, this, placeholders::_1, placeholders::_2));
	gateway->OnDeviceRPCCallbackRegister("DownloadFileResp", bind(&File::OnRPCDownloadFileResp, this, placeholders::_1, placeholders::_2));
}

bool File::uploadFile(string path, string name)
{
	LOGD("uploadFile");
	this->path = path;
	this->name = name;
	isBusy = true;
	string filePath = path + "/" + name;
	chunkIndex = 0;
	LOGD("uploadFile filePath: %s", filePath.c_str());

	ifstream uploadFile;
	uploadFile.open(filePath.c_str(), ios::in | ios::binary);
	if (uploadFile.is_open())
	{
		// Tính kích thước file
		uploadFile.seekg(0, ios::end);
		fileSize = uploadFile.tellg();
		uploadFile.seekg(0, ios::beg);
		uploadFile.close();

		chunkCount = fileSize / BIN_PACKAGE_SIZE;
		if (fileSize % BIN_PACKAGE_SIZE)
			++chunkCount;

		Json::Value jsonValue;
		Json::Value dataValue;
		dataValue["name"] = name;
		dataValue["path"] = path;
		dataValue["size"] = fileSize;
		dataValue["chunkSize"] = BIN_PACKAGE_SIZE;
		dataValue["chunkCount"] = chunkCount;
		dataValue["sum"] = "";
		dataValue["sumAlg"] = "md5";
		jsonValue["CMD"] = "UploadFile";
		jsonValue["DATA"] = dataValue;
		gateway->PublishToDeviceTelemetry(jsonValue);

		int timeout = chunkCount;
		while (chunkIndex < chunkCount && timeout--)
		{
			usleep(10000);
		}
	}
	else
	{
		LOGW("Open file %s error", filePath.c_str());
	}
	if (chunkIndex < chunkCount)
	{
		LOGW("upload file timeout");
	}
	else
	{
		LOGI("upload file done");
	}
	isBusy = false;
	return 0;
}

bool File::downloadFile(string path, string name)
{
	isBusy = true;
	string filePath = path + "/" + name;

	isBusy = false;
	return 0;
}

void File::OnFWMessage(string &topic, char *payload, int payloadlen)
{
}

bool File::UploadChunk()
{
	ifstream uploadFile;
	string filePath = path + "/" + name;
	LOGD("UploadChunk: %d, path: %s", chunkIndex * BIN_PACKAGE_SIZE, filePath.c_str());
	uploadFile.open(filePath.c_str(), ios::in | ios::binary);
	if (uploadFile.is_open())
	{
		uploadFile.seekg(chunkIndex * BIN_PACKAGE_SIZE, std::ios::beg);

		// Đọc nội dung file
		char file_content[BIN_PACKAGE_SIZE];
		int size = BIN_PACKAGE_SIZE;
		uploadFile.read(file_content, BIN_PACKAGE_SIZE);
		if (uploadFile.eof())
		{
			size = fileSize - chunkIndex * BIN_PACKAGE_SIZE;
		}
		uploadFile.close();

		gateway->CloudPublish(pubFwTopic + to_string(chunkIndex), file_content, size);
		return true;
	}
	return false;
}

int File::OnRPCUploadFileResp(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCUploadFileResp");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("code") && data["code"].isInt() &&
				data.isMember("state") && data["state"].isInt())
		{
			int code = data["code"].asInt();
			int state = data["state"].asInt();
			if (code == 0)
			{
				if (state == 0) // start
				{
					chunkIndex = 0;
					UploadChunk();
				}
				else if (state == 1) // done
				{
					chunkIndex = chunkCount + 1;
				}
			}
			else
			{
				LOGW("OnRPCUploadFileResp err: %d", code);
			}
		}
	}
	return 1;
}

int File::OnRPCUploadBinaryResp(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCUploadBinaryResp");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("code") && data["code"].isInt() &&
				data.isMember("chunk") && data["chunk"].isInt())
		{
			int code = data["code"].asInt();
			int chunk = data["chunk"].asInt();
			if (code == 0)
			{
				if (chunk == chunkIndex)
				{
					++chunkIndex;
					if (chunkIndex == chunkCount)
					{
						LOGD("Upload done");
					}
					else
					{
						UploadChunk();
					}
				}
			}
			else
			{
				LOGW("OnRPCUploadFileResp err: %d", code);
			}
		}
	}
	return 1;
}

int File::OnRPCDownloadFileResp(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDownloadFileResp");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
	}
	return 0;
}
