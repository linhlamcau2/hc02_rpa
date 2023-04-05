#pragma once

#include <string>
#include <map>
#include "json.h"
#include "File.h"

using namespace std;

class FileTransfer
{
public:
	FileTransfer();

	void init();

	int uploadFile(string path, string name);
	int uploadFile(File &file);
	int downloadFile(string path, string name);
	int downloadFile(File &file);
};

extern FileTransfer *fileTransfer;
