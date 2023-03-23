#include "FileTransfer.h"
#include <unistd.h>
#include "Gateway.h"
#include "Log.h"
#include "File.h"

FileTransfer *fileTransfer = NULL;

FileTransfer::FileTransfer()
{
	LOGD("FileTransfer");
}

void FileTransfer::init()
{
	LOGD("init");
	subRespTopicV2 = "v2/bin/resp/server/" + gateway->getMac();

	// gateway->cloudAddActionCallback(bind(&FileTransfer::OnFWMessage, this, placeholders::_1, placeholders::_2, placeholders::_3), subRespTopicV2);
	// gateway->OnDeviceRpcCallbackRegister("DownloadFileResp", bind(&FileTransfer::OnRpcDownloadFileResp, this, placeholders::_1, placeholders::_2));
}

int FileTransfer::uploadFile(string path, string name)
{
	File file(path, name);
	return uploadFile(file);
}

int FileTransfer::uploadFile(File &file)
{
	LOGD("uploadFile");
	string filePath = file.path + "/" + file.name;
	if (file.HaveInfo())
	{
		string sessionId = to_string(time(NULL)) + to_string(rand());
		Json::Value jsonValue;
		Json::Value dataValue;
		dataValue["name"] = file.name;
		dataValue["path"] = file.path;
		dataValue["size"] = file.fileSize;
		dataValue["chunkSize"] = BIN_PACKAGE_SIZE;
		dataValue["chunkCount"] = file.chunkCount;
		dataValue["sum"] = "";
		dataValue["sumAlg"] = "md5";
		dataValue["sessionId"] = sessionId;
		Json::Value respValue;
		int rs = gateway->PublishToCloudMessageV2("UploadFile", dataValue, "UploadFileResp", &respValue);
		if (rs == CODE_OK)
		{
			LOGD("uploadFile respValue: %s", respValue.toString().c_str());
			file.chunkIndex = 0;
			file.OpenToRead();
			if (file.IsOpen())
			{
				char fileContent[BIN_PACKAGE_SIZE];
				while (file.chunkIndex < file.chunkCount)
				{
					uint32_t size = file.Read(file.chunkIndex * BIN_PACKAGE_SIZE, fileContent, BIN_PACKAGE_SIZE);
					Json::Value respValue;
					int rs = gateway->PublishBinToCloudMessageV2(sessionId, file.chunkIndex, fileContent, size, "UploadBinResp", &respValue);
					if (rs == CODE_OK)
					{
						LOGD("UploadChunk respValue: %s", respValue.toString().c_str());
						++file.chunkIndex;
					}
					else
					{
						LOGD("UploadChunk err: %d", rs);
						break;
					}
				}
				file.Close();
			}
		}
	}
	else
	{
		LOGW("Open file %s error", filePath.c_str());
	}
	if (file.chunkIndex == file.chunkCount)
	{
		LOGI("upload file %s done", file.name.c_str());
		return CODE_OK;
	}
	return CODE_ERROR;
}

int FileTransfer::downloadFile(string path, string name)
{
	File file(path, name);
	return downloadFile(file);
}

int FileTransfer::downloadFile(File &file)
{
	LOGD("downloadFile");

	return CODE_ERROR;
}

void FileTransfer::OnFWMessage(string &topic, char *payload, int payloadlen)
{
}

int FileTransfer::OnRpcDownloadFileResp(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcDownloadFileResp");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
	}
	return CODE_ERROR;
}
