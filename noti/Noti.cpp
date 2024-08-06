#ifdef __ANDROID__
#include "Noti.h"
#include "Util.h"
#include "Log.h"
#include "Db.h"

Noti::Noti(string id, string type, string content, string updateTime, string createTime)
{
    this->id = id;
    this->type = type;
    this->content = content;
    this->isRead = false;
}

Noti::~Noti()
{
    LOGI("~Noti");
}

void Noti::UpdateNoti(bool isRead)
{
   this->isRead = isRead;
   database->NotiUpdate(this);
}

string Noti::GetId()
{
    return this->id;
}

string Noti::GetType()
{
    return this->type;
}

string Noti::GetContent()
{
    return this->content;
}

bool Noti::GetIsRead()
{
    return this->isRead;
}

#endif