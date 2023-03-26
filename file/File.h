#pragma once

#include <string>
#include <fstream>

#ifndef BIN_PACKAGE_SIZE //size of package can send in a second
#ifdef ESP_PLATFORM
#define BIN_PACKAGE_SIZE (4 * 1024)
#else
#define BIN_PACKAGE_SIZE (512 * 1024)
#endif
#endif

using namespace std;

class File
{
private:
public:
	string name;
	string path;
	string sumAlg;
	string sum;
	streamsize fileSize;
	string filePath;

	volatile int chunkIndex;
	int chunkCount;

	fstream file;
	bool haveInfo;

	File(string path, string name);

	virtual bool HaveInfo();

	virtual void Open(ios_base::openmode mode);
	virtual void Close();
	virtual void OpenToRead();
	virtual void OpenToWrite();
	virtual bool IsOpen();

	virtual int Read(char *buff, uint32_t size);
	virtual int Read(char *buff, uint32_t size, uint32_t position);
	virtual int Write(char *buff, uint32_t size);
	virtual int Write(char *buff, uint32_t size, uint32_t position);
};
