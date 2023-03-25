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
	subFwTopic = "/v1/server/hc/" + gateway->getMac() + "/bin/#";
	pubFwTopic = "/v1/hc/" + gateway->getMac() + "/bin/";
	gateway->cloudAddActionCallback(bind(&FileTransfer::OnFWMessage, this, placeholders::_1, placeholders::_2, placeholders::_3), subFwTopic);

	gateway->OnDeviceRpcCallbackRegister("UploadFileResp", bind(&FileTransfer::OnRpcUploadFileResp, this, placeholders::_1, placeholders::_2));
	gateway->OnDeviceRpcCallbackRegister("UploadBinaryResp", bind(&FileTransfer::OnRpcUploadBinaryResp, this, placeholders::_1, placeholders::_2));
	gateway->OnDeviceRpcCallbackRegister("DownloadFileResp", bind(&FileTransfer::OnRpcDownloadFileResp, this, placeholders::_1, placeholders::_2));
}

int FileTransfer::uploadFile(string path, string name)
{
	LOGD("uploadFile");
	string filePath = path + "/" + name;
	File file(path, name);
	if (file.HaveInfo())
	{
		string sessionId = to_string(time(NULL)) + to_string(rand());
		files[sessionId] = &file;
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
		jsonValue["CMD"] = "UploadFile";
		jsonValue["DATA"] = dataValue;
		gateway->PublishToDeviceTelemetry(jsonValue);

		int timeout = file.chunkCount;
		while (file.chunkIndex < file.chunkCount && timeout--)
		{
			sleep(1);
		}
		files.erase(sessionId);
	}
	else
	{
		LOGW("Open file %s error", filePath.c_str());
	}
	if (file.chunkIndex < file.chunkCount)
	{
		LOGW("upload file timeout");
	}
	else
	{
		LOGI("upload file %s done", name.c_str());
	}
	return CODE_OK;
}

int FileTransfer::downloadFile(string path, string name)
{
	// isBusy = true;
	// string filePath = path + "/" + name;

	// isBusy = false;
	return CODE_OK;
}

void FileTransfer::OnFWMessage(string &topic, char *payload, int payloadlen)
{
}

int FileTransfer::UploadChunk(string sessionId, File *file)
{
	string filePath = file->path + "/" + file->name;
	LOGD("UploadChunk: %d, path: %s", file->chunkIndex * BIN_PACKAGE_SIZE, filePath.c_str());
	file->OpenToRead();
	if (file->IsOpen())
	{
		char *fileContent = (char *)malloc(BIN_PACKAGE_SIZE);
		if (fileContent)
		{
			uint32_t size = file->Read(file->chunkIndex * BIN_PACKAGE_SIZE, fileContent, BIN_PACKAGE_SIZE);
			file->Close();
			gateway->CloudPublish(pubFwTopic + sessionId + "/" + to_string(file->chunkIndex), fileContent, size);
			free(fileContent);
			return CODE_OK;
		}
		else
		{
			LOGW("malloc err");
		}
	}
	return CODE_ERROR;
}

int FileTransfer::OnRpcUploadFileResp(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcUploadFileResp");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("code") && data["code"].isInt() &&
				data.isMember("state") && data["state"].isInt() &&
				data.isMember("sessionId") && data["sessionId"].isString())
		{
			int code = data["code"].asInt();
			int state = data["state"].asInt();
			string sessionId = data["sessionId"].asString();
			if (files.find(sessionId) != files.end())
			{
				File *file = files[sessionId];
				if (code == 0)
				{
					if (state == 0) // start
					{
						file->chunkIndex = 0;
						UploadChunk(sessionId, file);
					}
					else if (state == 1) // done
					{
					}
				}
				else
				{
					LOGW("OnRpcUploadFileResp err: %d", code);
				}
			}
			else
			{
				LOGW("OnRpcUploadFileResp session not found: %s", sessionId.c_str());
			}
		}
	}
	return CODE_ERROR;
}

int FileTransfer::OnRpcUploadBinaryResp(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcUploadBinaryResp");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
		Json::Value data = reqValue["DATA"];
		if (data.isMember("code") && data["code"].isInt() &&
				data.isMember("chunk") && data["chunk"].isInt() &&
				data.isMember("sessionId") && data["sessionId"].isString())
		{
			int code = data["code"].asInt();
			int chunk = data["chunk"].asInt();
			string sessionId = data["sessionId"].asString();
			if (files.find(sessionId) != files.end())
			{
				File *file = files[sessionId];
				if (code == 0)
				{
					if (chunk == file->chunkIndex)
					{
						++file->chunkIndex;
						if (file->chunkIndex == file->chunkCount)
						{
							LOGD("Upload done");
						}
						else
						{
							UploadChunk(sessionId, file);
						}
					}
				}
				else
				{
					LOGW("OnRpcUploadFileResp err: %d", code);
				}
			}
		}
	}
	return CODE_ERROR;
}

int FileTransfer::OnRpcDownloadFileResp(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRpcDownloadFileResp");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
	}
	return CODE_ERROR;
}
