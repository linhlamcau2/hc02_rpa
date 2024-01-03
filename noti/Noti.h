#pragma once

#include <string>
#include <vector>
#include "json.h"
#include <unistd.h>

using namespace std;

typedef enum
{
	NOTI_HOME = -2,
	NOTI_WARNING = -1,
	MOTI_DEFAULT
} NotiType;

class Noti
{
private:
    string id;
    string type;
    string content;
    string updateTime;
    string createTime;
    bool isRead;

public:
    Noti(string id, string type, string content, string updateTime, string createTime);
    ~Noti();
    void UpdateNoti(bool isRead);
    string GetId();
    string GetType();
    string GetContent();
    bool GetIsRead();
};