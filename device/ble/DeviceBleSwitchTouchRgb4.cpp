#include "DeviceBleSwitchTouchRgb4.h"
#include "Log.h"

DeviceBleSwitchTouchRgb4::DeviceBleSwitchTouchRgb4(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_SWITCH_4, version)
{
	for (int i = 0; i < 4; i++)
	{
		elementButton[i] = new ElementButton(this, addr + i);
		elementRgb[i] = new ElementRgb(this, addr + i);
	}
	powerSource = POWER_AC;
}

bool DeviceBleSwitchTouchRgb4::CheckAddr(uint32_t addr)
{
	return this->addr <= addr && this->addr + 4 > addr;
}

int DeviceBleSwitchTouchRgb4::BuildTelemetryValue(Json::Value &pushDataValue)
{
	for (int i = 0; i < 4; i++)
	{
		elementButton[i]->BuildTelemetryValue(pushDataValue);
		elementRgb[i]->BuildTelemetryValue(pushDataValue);
	}
	return 0;
}

int DeviceBleSwitchTouchRgb4::BuildTelemetryValueV2(Json::Value &pushDataValue)
{
	for (int i = 0; i < 4; i++)
	{
		elementButton[i]->BuildTelemetryValueV2(pushDataValue);
		elementRgb[i]->BuildTelemetryValueV2(pushDataValue);
	}
	return 0;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void DeviceBleSwitchTouchRgb4::InitAttribute(int attributeId, double value)
{
	for (int i = 0; i < 4; i++)
	{
		elementButton[i]->InitAttribute(attributeId, value);
		elementRgb[i]->InitAttribute(attributeId, value);
	}
}
#endif

void DeviceBleSwitchTouchRgb4::InputData(uint8_t *data, int len, uint32_t addr)
{
	if (!CheckAddr(addr))
		return;
	values = Json::Value::null;
	if (!elementButton[addr - this->addr]->InputData(data, len, values))
	{
		if (!elementRgb[addr - this->addr]->InputData(data, len, values))
		{
			return;
		}
	}
	PushTelemetry(values);
}

bool DeviceBleSwitchTouchRgb4::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	for (int i = 0; i < 4; i++)
	{
		if (elementButton[i]->CheckData(dataValue, rs))
		{
			return true;
		}
	}
	return false;
}

// TODO: viet anh recheck DoJsonArray
bool DeviceBleSwitchTouchRgb4::DoJsonArray(Json::Value &dataValue)
{
	LOGD("DoJsonArray data: %s", dataValue.toString().c_str());
	if (dataValue.isObject())
	{
		for (int j = 0; j < 4; j++)
		{
			if (elementButton[j]->Do(dataValue))
				return true;
		}
	}
	else if (dataValue.isArray())
	{
		for (int j = 0; j < 4; j++)
		{
			if (elementRgb[j]->DoJsonArray(dataValue))
				return true;
		}
	}
	return false;
}

bool DeviceBleSwitchTouchRgb4::DoV2(Json::Value &dataValue)
{
	for (int j = 0; j < 4; j++)
	{
		elementButton[j]->DoV2(dataValue);
		elementRgb[j]->DoV2(dataValue);
	}
	return true;
}
