#include "DeviceBleOnoffCctDim.h"
#include <Log.h>

DeviceBleOnoffCctDim::DeviceBleOnoffCctDim(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, type, version)
{
	elementOnOff = new ElementOnOff(this, addr);
	elementCct = new ElementCct(this, addr + 1);
	elementDim = new ElementDim(this, addr);
}

bool DeviceBleOnoffCctDim::CheckAddr(uint32_t addr)
{
	return ((this->addr <= addr) && (this->addr + 1 >= addr));
}

int DeviceBleOnoffCctDim::BuildTelemetryValue(Json::Value &pushDataValue)
{
	elementOnOff->BuildTelemetryValue(pushDataValue);
	elementCct->BuildTelemetryValue(pushDataValue);
	elementDim->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleOnoffCctDim::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (!elementOnOff->InputData(data, len, values))
	{
		if (!elementCct->InputData(data, len, values))
		{
			if (!elementDim->InputData(data, len, values))
			{
				return;
			}
		}
	}
	PushTelemetry(values);
}

bool DeviceBleOnoffCctDim::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (!elementOnOff->CheckData(dataValue, rs))
	{
		if (!elementCct->CheckData(dataValue, rs))
		{
			if (!elementDim->CheckData(dataValue, rs))
			{
				return false;
			}
		}
	}
	return true;
}

bool DeviceBleOnoffCctDim::Do(Json::Value &dataValue)
{
	if (!elementOnOff->Do(dataValue))
	{
		if (!elementCct->Do(dataValue))
		{
			if (!elementDim->Do(dataValue))
			{
				return false;
			}
		}
	}
	return true;
}
