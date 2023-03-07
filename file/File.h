#pragma once

#include <string>
#include <fstream>

#ifndef BIN_PACKAGE_SIZE
#define BIN_PACKAGE_SIZE 4096
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

	volatile int chunkIndex;
	int chunkCount;

	fstream file;
	bool haveInfo;

	File(string path, string name);

	bool HaveInfo();

	void Open(ios_base::openmode mode);
	void OpenToRead();
	void OpenToWrite();
};
