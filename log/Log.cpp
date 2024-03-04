
/*
 * log.c
 *
 *  Created on: Jan 5, 2019
 *      Author: Thinpv
 */

#include <stdio.h>
#include <string.h>
#include "Log.h"
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <ctime>
#include <filesystem>
#include <chrono>
#include <map>
#include <regex>
#include <unistd.h>

using namespace std::chrono;

const long maxLogFileSize = 2 * 1024 * 1024;
const int maxNumLogFiles = 5;

bool checkLogFileSize(const char *filePath)
{
	return std::__fs::filesystem::exists(filePath) && std::__fs::filesystem::file_size(filePath) < maxLogFileSize;
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

bool isLogFile(const std::__fs::filesystem::directory_entry &entry)
{
	std::string filename = entry.path().filename().string();
	return filename.find(LOG_FILE_NAME) == 0 && filename.find(".log") != std::string::npos;
}

bool isValidDateFormat(const std::string &dateString)
{
	static const std::regex dateRegex("[0-9]{8}");
	return std::regex_match(dateString, dateRegex);
}

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
	va_end(list);

	//Log to File

	time_t rawtime;
	struct tm *timeinfo;
	char timeBuffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(timeBuffer, sizeof(timeBuffer), "[%Y-%m-%d %H:%M:%S]", timeinfo);

	char dataBuffer[80];
	strftime(dataBuffer, sizeof(dataBuffer), "%Y%m%d", timeinfo);

		std::string logFilename = std::string(LOG_FILE_PATH) + std::string(LOG_FILE_NAME) + std::string(dataBuffer) + ".log";

	std::ofstream logFile(logFilename, std::ios::app);

	if (!checkLogFileSize(logFilename.c_str()))
	{
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
	va_end(args);

	std::string logFilename = std::string(LOG_FILE_PATH) + std::string(LOG_FILE_NAME) + std::string(dataBuffer) + ".log";

	std::ofstream logFile(logFilename, std::ios::app);

	if (!checkLogFileSize(logFilename.c_str()))
	{
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
}

#endif /* __ANDROID__ */

void checkLogFile()
{

	std::string currentDate = getCurrentDate();

	std::multimap<std::string, std::__fs::filesystem::directory_entry> logFiles;

	for (const auto &entry : std::__fs::filesystem::directory_iterator(LOG_FILE_PATH))
	{
		if (std::__fs::filesystem::is_regular_file(entry))
		{
			std::string filename = entry.path().filename().string();
			std::string dateString = filename.substr(4, 8);

			if (isValidDateFormat(dateString))
				logFiles.emplace(dateString, entry);
		}
	}

	int logCount = 0;
	for (auto it = logFiles.rbegin(); it != logFiles.rend(); ++it)
	{
		if (logCount >= maxNumLogFiles)
		{
			std::__fs::filesystem::remove(it->second.path());
			std::cout << "Removed old log file: " << it->second.path().filename() << std::endl;
		}
		else
		{
			logCount++;
		}
	}
}

static time_t oldTime;
void threadCheckLog()
{
	while (1)
	{
		if (time(nullptr) - oldTime >= 82800)
		{
			oldTime = time(nullptr);
			checkLogFile();
		}
		else
			sleep(1);
	}
}