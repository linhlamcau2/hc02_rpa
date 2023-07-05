#include "FileTransfer.h"
#include <unistd.h>
#include "Gateway.h"
#include "Log.h"
#include "File.h"
#include "Util.h"

FileTransfer *fileTransfer = NULL;

FileTransfer::FileTransfer()
{
	LOGD("FileTransfer");
}

void FileTransfer::init()
{
	LOGD("init");
}

int FileTransfer::uploadFile(string path, string name)
{
	File file(path, name);
	return uploadFile(file);
}

int FileTransfer::uploadFile(File &file)
{
	LOGD("uploadFile");
	int rs = CODE_ERROR;
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
		rs = gateway->PublishToCloudMessage("UploadFile", dataValue, "UploadFileResp", &respValue);
		if (rs == CODE_OK)
		{
			LOGD("uploadFile respValue: %s", respValue.toString().c_str());
			file.OpenToRead();
			if (file.IsOpen())
			{
				char fileContent[BIN_PACKAGE_SIZE];
				file.chunkIndex = 0;
				while (file.chunkIndex < file.chunkCount)
				{
					uint32_t size = file.Read(fileContent, BIN_PACKAGE_SIZE);
					Json::Value respValue;
					rs = gateway->PublishBinToCloudMessage(sessionId, file.chunkIndex, fileContent, size, "UploadBinResp", &respValue);
					if (rs == CODE_OK)
					{
						LOGD("UploadChunk respValue: %s", respValue.toString().c_str());
						++file.chunkIndex;
					}
					else
					{
						LOGD("UploadChunk err: %d", rs);
						rs = CODE_ERROR;
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
	if (rs == CODE_OK)
	{
		LOGI("upload file %s done", file.name.c_str());
	}
	else
	{
		LOGW("upload file %s err: %d", file.name.c_str(), rs);
	}
	return rs;
}

int FileTransfer::downloadFile(string path, string name)
{
	File file(path, name);
	return downloadFile(file);
}

int FileTransfer::downloadFile(File &file)
{
	LOGD("downloadFile");
	int rs = CODE_ERROR;
	string sessionId = to_string(time(NULL)) + to_string(rand());
	Json::Value jsonValue;
	Json::Value dataValue;
	dataValue["name"] = file.name;
	dataValue["path"] = file.path;
	dataValue["chunkSize"] = BIN_PACKAGE_SIZE;
	dataValue["sumAlg"] = "md5";
	dataValue["sessionId"] = sessionId;
	Json::Value respValue;
	rs = gateway->PublishToCloudMessage("DownloadFile", dataValue, "DownloadFileResp", &respValue);
	if (rs == CODE_OK)
	{
		LOGD("DownloadFile respValue: %s", respValue.toString().c_str());
		if (respValue.isMember("code") && respValue["code"].isInt() &&
				respValue.isMember("size") && respValue["size"].isInt())
		{
			rs = respValue["code"].asInt();
			if (rs == CODE_OK)
			{
				string cmd = "rm " + file.filePath;
				Util::ExecuteCMD(cmd.c_str());
				LOGD("cmd: %s", cmd.c_str());

				file.fileSize = respValue["size"].asInt();
				file.chunkCount = file.fileSize / BIN_PACKAGE_SIZE;
				if (file.fileSize % BIN_PACKAGE_SIZE)
					++file.chunkCount;
				file.haveInfo = true;
				LOGD("fileSize: %d, chunkCount: %d", (int)file.fileSize, file.chunkCount);
				file.OpenToWrite();
				if (file.IsOpen())
				{
					file.chunkIndex = 0;
					while (file.chunkIndex < file.chunkCount)
					{
						Json::Value jsonValue;
						Json::Value dataValue;
						dataValue["chunk"] = file.chunkIndex;
						dataValue["sessionId"] = sessionId;
						string rqi = sessionId + to_string(file.chunkIndex);
						char payload[BIN_PACKAGE_SIZE];
						int payloadLen = BIN_PACKAGE_SIZE;
						rs = gateway->PublishToCloudRecieveBinMessage("DownloadBin", dataValue, rqi, payload, &payloadLen);
						if (rs == CODE_OK)
						{
							file.Write(payload, payloadLen);
							++file.chunkIndex;
						}
						else
						{
							LOGW("DownloadBin index %d err: %d", file.chunkIndex, rs);
						}
					}
					file.Close();
				}
			}
			else
			{
				LOGW("Cannot download file");
			}
		}
	}
	if (rs == CODE_OK)
	{
		LOGI("Download file %s done", file.name.c_str());
		LOGW("Need check sum");
	}
	else
	{
		LOGW("Download file %s err: %d", file.name.c_str(), rs);
	}
	return CODE_ERROR;
}
