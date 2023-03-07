#include "FileTransfer.h"
#include <fstream>
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

	gateway->OnDeviceRPCCallbackRegister("UploadFileResp", bind(&FileTransfer::OnRPCUploadFileResp, this, placeholders::_1, placeholders::_2));
	gateway->OnDeviceRPCCallbackRegister("UploadBinaryResp", bind(&FileTransfer::OnRPCUploadBinaryResp, this, placeholders::_1, placeholders::_2));
	gateway->OnDeviceRPCCallbackRegister("DownloadFileResp", bind(&FileTransfer::OnRPCDownloadFileResp, this, placeholders::_1, placeholders::_2));
}

bool FileTransfer::uploadFile(string path, string name)
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
			usleep(10000);
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
	return 0;
}

bool FileTransfer::downloadFile(string path, string name)
{
	// isBusy = true;
	// string filePath = path + "/" + name;

	// isBusy = false;
	return 0;
}

void FileTransfer::OnFWMessage(string &topic, char *payload, int payloadlen)
{
}

bool FileTransfer::UploadChunk(string sessionId, File *file)
{
	ifstream uploadFile;
	string filePath = file->path + "/" + file->name;
	LOGD("UploadChunk: %d, path: %s", file->chunkIndex * BIN_PACKAGE_SIZE, filePath.c_str());
	uploadFile.open(filePath.c_str(), ios::in | ios::binary);
	if (uploadFile.is_open())
	{
		uploadFile.seekg(file->chunkIndex * BIN_PACKAGE_SIZE, std::ios::beg);

		// Đọc nội dung file
		char file_content[BIN_PACKAGE_SIZE];
		int size = BIN_PACKAGE_SIZE;
		uploadFile.read(file_content, BIN_PACKAGE_SIZE);
		if (uploadFile.eof())
		{
			size = file->fileSize - file->chunkIndex * BIN_PACKAGE_SIZE;
		}
		uploadFile.close();

		gateway->CloudPublish(pubFwTopic + sessionId + "/" + to_string(file->chunkIndex), file_content, size);
		return true;
	}
	return false;
}

int FileTransfer::OnRPCUploadFileResp(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCUploadFileResp");
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
					LOGW("OnRPCUploadFileResp err: %d", code);
				}
			}
			else
			{
				LOGW("OnRPCUploadFileResp session not found: %s", sessionId.c_str());
			}
		}
	}
	return 1;
}

int FileTransfer::OnRPCUploadBinaryResp(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCUploadBinaryResp");
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
					LOGW("OnRPCUploadFileResp err: %d", code);
				}
			}
		}
	}
	return 1;
}

int FileTransfer::OnRPCDownloadFileResp(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnRPCDownloadFileResp");
	if (reqValue.isMember("DATA") && reqValue["DATA"].isObject())
	{
	}
	return 0;
}
