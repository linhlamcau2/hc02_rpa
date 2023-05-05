#include "AttributeOnoff.h"
#include "Log.h"
#include "Util.h"
#include "zigbee/cluster/Cluster.h"
#include "Device.h"
#include "Db.h"

AttributeOnoff::AttributeOnoff(Cluster *cluster) : Attribute(cluster)
{
}

void AttributeOnoff::InitAttribute(int attributeId, double value)
{
	// if (attributeId == parameterToId["onoff"])
	// 	onoff = value;
}

void AttributeOnoff::SaveAttribute()
{
	// database->DeviceAttributeAddOrReplace(cluster->getDevice(), parameterToId["onoff1"], onoff);
}

void AttributeOnoff::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
	uint8_t dataType = data[0];
	switch (dataType)
	{
	case 0x10:
		onoff = data[1];
		SaveAttribute();
		BuildTelemetryValue(jsonValue);
		if (cluster && cluster->getDevice())
		{
			cluster->getDevice()->CheckTrigger();
		}
		break;

	default:
		break;
	}
}

bool AttributeOnoff::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember("onoff") && dataValue["onoff"].isInt())
		{
			int onoff = dataValue["onoff"].asInt();
			rs = Util::CompareNumber(this->onoff, onoff, 0, op);
			return true;
		}
	}
	return false;
}

void AttributeOnoff::BuildTelemetryValue(Json::Value &jsonValue)
{
	// Json::Value dataValue;
	// dataValue["ID"] = parameterToId["onoff"];
	// dataValue["VALUE"] = onoff;
	// jsonValue.append(dataValue);
}
