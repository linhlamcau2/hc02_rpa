/*
 * log.c
 *
 *  Created on: Jan 5, 2019
 *      Author: Thinpv
 */
#include "Log.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <regex>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <cstdarg>

#ifndef ESP_PLATFORM
using namespace std::chrono;

#ifdef __ANDROID__
const long maxLogFileSize = 4 * 1024 * 1024;
const int maxNumLogFiles = 7;
#else
const long maxLogFileSize = 2 * 1024 * 1024;
const int maxNumLogFiles = 3;
#endif

bool checkLogFileSize(std::fstream &file)
{
	if (file.is_open())
	{
		file.seekg(0, std::ios::end);
		long size = file.tellg();
		file.seekg(0, std::ios::beg);

		if (size < maxLogFileSize)
			return true;
	}
	return false;
}

std::string getCurrentDate()
{
	auto now = system_clock::now();
	auto now_time = system_clock::to_time_t(now);
	std::tm *timeinfo = std::localtime(&now_time);

	char buffer[9];
	std::strftime(buffer, sizeof(buffer), "%Y%m%d", timeinfo);

	return std::string(buffer);
}

bool isValidDateFormat(const std::string &dateString)
{
	static const std::regex dateRegex("[0-9]{8}");
	return std::regex_match(dateString, dateRegex);
}

#endif //

#ifndef __ANDROID__

static vprintf_like_t s_log_print_func = &vprintf;

log_level_t log_level = LOG_INFO;

static char time_str[16];
char *timestr()
{
	time_t t = time(NULL);
	struct tm *timeinfo;
	timeinfo = localtime(&t);
	sprintf(time_str, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	return time_str;
}

void log_set_level(log_level_t log_level_)
{
	log_level = log_level_;
}

void log_set_vprintf(vprintf_like_t func)
{
	s_log_print_func = func;
}

void log_write(const char *format, ...)
{

	va_list list;
	va_start(list, format);
	(*s_log_print_func)(format, list);

#ifdef __OPENWRT__
	// Log to File
	time_t rawtime;
	struct tm *timeinfo;
	char timeBuffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(timeBuffer, sizeof(timeBuffer), "[%Y-%m-%d %H:%M:%S]", timeinfo);

	char dataBuffer[80];
	strftime(dataBuffer, sizeof(dataBuffer), "%Y%m%d", timeinfo);

	std::string logFilename = std::string(LOG_FILE_PATH) + std::string(LOG_FILE_NAME) + std::string(dataBuffer) + ".log";

	std::fstream logFile(logFilename, std::ios::app);

	if (!checkLogFileSize(logFile))
	{
		va_end(list);
		return;
	}

	if (logFile.is_open())
	{
		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "%s ", timeBuffer);
		vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), format, list);
		logFile << buffer << std::endl;
		logFile.close();
	}
	else
		std::cout << "open file error" << std::endl;
#endif
	va_end(list);
}

char *log_cut_str(char *full_path, uint8_t len)
{
	uint8_t k;
	char *ptr;
	k = strlen(full_path);

	if (k <= len)
		return full_path;

	ptr = full_path + (k - len);
	return ptr;
}
#else

#include <android/log.h>

void logPrint(int priority, const char *tag, const char *format, ...)
{

	time_t rawtime;
	struct tm *timeinfo;
	char timeBuffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(timeBuffer, sizeof(timeBuffer), "[%Y-%m-%d %H:%M:%S]", timeinfo);

	char dataBuffer[80];
	strftime(dataBuffer, sizeof(dataBuffer), "%Y%m%d", timeinfo);

	va_list args;
	va_start(args, format);
	__android_log_vprint(priority, tag, format, args);

	std::string logFilename = std::string(LOG_FILE_PATH) + std::string(LOG_FILE_NAME) + std::string(dataBuffer) + ".log";

	std::fstream logFile(logFilename, std::ios::app);

	if (!checkLogFileSize(logFile))
	{
		va_end(args);
		return;
	}

	if (logFile.is_open())
	{
		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "%s ", timeBuffer);
		vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), format, args);
		logFile << buffer << std::endl;
		logFile.close();
	}
	else
		std::cout << "open file error" << std::endl;
	va_end(args);
}

#endif /* ANDROID */

#ifndef ESP_PLATFORM
bool compareDates(const std::pair<std::string, std::string> &a, const std::pair<std::string, std::string> &b)
{
	return a.first < b.first;
}

void checkLogFile()
{

	std::string currentDate = getCurrentDate();

	std::vector<std::pair<std::string, std::string>> logFiles;

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(std::string(LOG_FILE_PATH).c_str())) != nullptr)
	{
		while ((ent = readdir(dir)) != nullptr)
		{
			if (ent->d_type == DT_REG)
			{
				std::string filename = ent->d_name;
				std::string dateString = filename.substr(4, 8);

				if (isValidDateFormat(dateString))
					logFiles.emplace_back(dateString, filename);
			}
		}
		closedir(dir);
	}
	else
	{
		std::cerr << "Error opening directory" << std::endl;
		return;
	}

	std::sort(logFiles.begin(), logFiles.end(), compareDates);

	int logCount = 0;
	for (auto it = logFiles.rbegin(); it != logFiles.rend(); ++it)
	{
		if (logCount >= maxNumLogFiles)
		{
			std::string filePath = LOG_FILE_PATH + it->second;
			if (remove(filePath.c_str()) == 0)
			{
				std::cout << "Removed old log file: " << it->second << std::endl;
			}
			else
			{
				std::cerr << "Error removing old log file: " << it->second << std::endl;
			}
		}
		else
		{
			logCount++;
		}
	}
}
#endif
