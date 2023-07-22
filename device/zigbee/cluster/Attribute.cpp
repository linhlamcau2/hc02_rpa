#include "Attribute.h"
#include "Cluster.h"
#include "Device.h"

Attribute::Attribute(uint16_t id, Cluster *cluster)
{
	this->id = id;
	this->cluster = cluster;
}

void Attribute::CheckTrigger()
{
	LOGV("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : cluster->getDevice()->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

bool Attribute::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGW("CheckData not implement");
	return false;
}
