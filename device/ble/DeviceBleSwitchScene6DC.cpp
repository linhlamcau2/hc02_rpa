#include "DeviceBleSwitchScene6DC.h"
#include <Log.h>

DeviceBleSwitchScene6DC::DeviceBleSwitchScene6DC(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_DC_SCENE_CONTACT, version)
{
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i] = new ModuleButton(this, addr, i);
	}
	modulePinLevel = new ModulePinLevel(this, addr);
	powerSource = POWER_BATTERY;
}

int DeviceBleSwitchScene6DC::BuildTelemetryValue(Json::Value &pushDataValue)
{
	modulePinLevel->BuildTelemetryValue(pushDataValue);
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i]->BuildTelemetryValue(pushDataValue);
	}
	return 0;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void DeviceBleSwitchScene6DC::InitAttribute(int attributeId, double value)
{
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i]->InitAttribute(attributeId, value);
	}
}
#endif

void DeviceBleSwitchScene6DC::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	for (int i = 0; i < 6; i++)
	{
		if (moduleButton[i]->InputData(data, len, values))
		{
			PushTelemetry(values);
			return;
		}
	}
	if (modulePinLevel->InputData(data, len, values))
	{
		PushTelemetry(values);
		return;
	}
}

bool DeviceBleSwitchScene6DC::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	return false;
}

bool DeviceBleSwitchScene6DC::Do(int id, int value)
{
	LOGD("DoTrigger id: %d, value: %d", id, value);
	return false;
}
