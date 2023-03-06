#include "File.h"
#include "Gateway.h"

File::File()
{

}

void File::init()
{
	string subInfoTopic = "/v1/server/hc/" + gateway->getMac() + "/fw/json";
	string subBinTopic = "/v1/server/hc/" + gateway->getMac() + "/fw/bin/+";
	gateway->cloudAddActionCallback(bind(&File::OnFWInfoMessage, this, placeholders::_1, placeholders::_2), subInfoTopic);
	gateway->cloudAddActionCallback(bind(&File::OnFWBinMessage, this, placeholders::_1, placeholders::_2), subBinTopic);
}

void File::OnFWBinMessage(string &topic, string &payload)
{

}

void File::OnFWInfoMessage(string &topic, string &payload)
{

}

bool File::uploadFile(string name, string url, string sum)
{
	return 0;
}

bool File::downloadFile(string name, string url, string sum)
{
	return 0;
}
