#pragma once

#include <string>

using namespace std;

class File
{
public:
	File();

	void init();

	bool uploadFile(string name, string url, string sum);
	bool downloadFile(string name, string url, string sum);

private:
	void OnFWBinMessage(string &topic, string &payload);
	void OnFWInfoMessage(string &topic, string &payload);
};