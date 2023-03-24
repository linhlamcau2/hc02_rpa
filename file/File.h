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

	bool HaveInfo();

	void Open(ios_base::openmode mode);
	void Close();
	void OpenToRead();
	void OpenToWrite();
	bool IsOpen();

	int Read(char *buff, uint32_t size);
	int Read(char *buff, uint32_t size, uint32_t position);
	int Write(char *buff, uint32_t size);
	int Write(char *buff, uint32_t size, uint32_t position);
};
