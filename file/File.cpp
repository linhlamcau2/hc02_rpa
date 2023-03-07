#include "File.h"
#include <fstream>
#include "Log.h"

File::File(string path, string name)
{
	this->path = path;
	this->name = name;
	haveInfo = false;
	chunkIndex = 0;

	string filePath = path + "/" + name;
	fstream uploadFile;
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

		haveInfo = true;
	}
}

bool File::HaveInfo()
{
	return haveInfo;
}

void File::Open(ios_base::openmode mode)
{
	string filePath = path + "/" + name;
	file.open(filePath.c_str(), mode);
}

void File::OpenToRead()
{
	Open(ios::in | ios::binary);
}

void File::OpenToWrite()
{
	Open(ios::app | ios::out | ios::binary);
}
