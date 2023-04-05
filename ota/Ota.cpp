#include <unistd.h>
#include <dirent.h>
#include "Ota.h"
#include "Util.h"
#include "Log.h"
#include "Http.h"

void Ota::init()
{
}

int Ota::startOta(string name, string url, string sum)
{
	// Update opkg
	system("opkg update");
	sleep(30);

	// Download file
	string wget = "wget " + string(BASE_URL_DEV) + url;
	system(wget.c_str());
	sleep(10);

	std::size_t last_slash = url.find_last_of("/");
	std::string filename = url.substr(last_slash + 1);
	std::size_t last_dot = filename.find_last_of(".");
	std::string extentsion = filename.substr(last_dot + 1);
	string cmdGetChecksum = "sha256sum " + filename;

	if (access(filename.c_str(), F_OK) != -1)
	{
		string checkSumCmd = Util::ExecuteCMD(cmdGetChecksum.c_str());
		string checkSum = checkSumCmd.substr(0, checkSumCmd.find(" "));
		LOGD("checksumGet: %s,checksumCal: %s", checkSum.c_str(), sum.c_str());
		if (checkSum == sum)
		{
			string tar = "tar -xJf " + filename;
			string fileTar = filename.substr(0, filename.find(".tar.xz"));
			system(tar.c_str());
			if (access(fileTar.c_str(), F_OK) != -1)
			{
				// Kiem tra co ton tai file .ipk trong folder
				string dir_path = "/root/" + fileTar;
				DIR *dir = opendir(dir_path.c_str());
				if (dir)
				{
					struct dirent *entry;
					while ((entry = readdir(dir)) != NULL)
					{
						if (entry->d_type == DT_REG && strcmp(strrchr(entry->d_name, '.'), ".ipk") == 0)
						{
							string install = "/usr/bin/nohup /bin/opkg install " + dir_path + "/" + string(entry->d_name) + "> /log.txt 2>&1 &";
							std::cout << install << std::endl;
							
							system(install.c_str());
						}
					}
					closedir(dir);
				}
				else
				{
					LOGW("File ipk not found");
				}
			}
			else
			{
				LOGW("TAR file error");
			}
		}
		else
		{
			LOGW("Checksum not matched");
		}
	}
	else
	{
		LOGW("File download does not exist");
		return CODE_OK;
	}

	return CODE_OK;
}
