#include "File.h"
#include <fstream>
#include "Log.h"

File::File(string path, string name)
{
	this->path = path;
	this->name = name;
	haveInfo = false;
	chunkIndex = 0;
	filePath = path + "/" + name;
}

bool File::HaveInfo()
{
	if (!haveInfo)
	{
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
	return haveInfo;
}

void File::Open(ios_base::openmode mode)
{
	file.open(filePath.c_str(), mode);
}

void File::Close()
{
	file.close();
}

void File::OpenToRead()
{
	Open(ios::in | ios::binary);
}

void File::OpenToWrite()
{
	Open(ios::app | ios::out | ios::binary);
}

bool File::IsOpen()
{
	return file.is_open();
}

int File::Read(char *buff, uint32_t size)
{
	uint32_t rs = size;
	streampos position = file.tellg();
	file.read(buff, size);
	if (file.eof())
	{
		rs = fileSize - position;
	}
	return rs;
}

int File::Read(char *buff, uint32_t size, uint32_t position)
{
	uint32_t rs = size;
	file.seekg(position, std::ios::beg);
	file.read(buff, size);
	if (file.eof())
	{
		rs = fileSize - position;
	}
	return rs;
}

int File::Write(char *buff, uint32_t size)
{
	file.write(buff, size);
	return size;
}

int File::Write(char *buff, uint32_t size, uint32_t position)
{
	uint32_t rs = size;
	file.seekg(position, std::ios::beg);
	file.write(buff, size);
	return rs;
}
