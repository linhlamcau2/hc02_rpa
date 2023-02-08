#include "DeviceBleSwitch4.h"
#include <Log.h>

DeviceBleSwitch4::DeviceBleSwitch4(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_SWITCH_4, version)
{
	for (int i = 0; i < 4; i++)
	{
		elementOnOff[i] = new ElementOnOff(this, addr + i);
	}
}

bool DeviceBleSwitch4::CheckAddr(uint32_t addr)
{
	return this->addr <= addr && this->addr + 4 > addr;
}

int DeviceBleSwitch4::BuildTelemetryValue(Json::Value &pushDataValue)
{
	for (int i = 0; i < 4; i++)
	{
		elementOnOff[i]->BuildTelemetryValue(pushDataValue);
	}
	return 0;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void DeviceBleSwitch4::InitAttribute(int attributeId, double value)
{
	for (int i = 0; i < 4; i++)
	{
		elementOnOff[i]->InitAttribute(attributeId, value);
	}
}
#endif

void DeviceBleSwitch4::InputData(uint8_t *data, int len, uint32_t addr)
{
	if (!CheckAddr(addr))
		return;
	values = Json::Value::null;
	if (!elementOnOff[addr - this->addr]->InputData(data, len, values))
	{
		return;
	}
	PushTelemetry(values);
}

bool DeviceBleSwitch4::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	for (int i = 0; i < 4; i++)
	{
		if (elementOnOff[i]->CheckData(dataValue, rs))
		{
			return true;
		}
	}
	return false;
}

bool DeviceBleSwitch4::Do(Json::Value &dataValue)
{
	// LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
	{
		Json::Value property = dataValue[i];
		for (int j = 0; j < 4; j++)
		{
			if (elementOnOff[j]->Do(property))
				return true;
		}
	}
	return false;
}
