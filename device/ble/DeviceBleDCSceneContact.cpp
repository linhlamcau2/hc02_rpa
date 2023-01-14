#include "DeviceBleDCSceneContact.h"
#include <Log.h>

DeviceBleDCSceneContact::DeviceBleDCSceneContact(string id, string name, string mac,string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_DC_SCENE_CONTACT, version)
{
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i] = new ModuleButton(this, i);
	}
	modulePinLevel = new ModulePinLevel(this);
}

int DeviceBleDCSceneContact::BuildTelemetryValue(Json::Value &pushDataValue)
{
	modulePinLevel->BuildTelemetryValue(pushDataValue);
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i]->BuildTelemetryValue(pushDataValue);
	}
	return 0;
}

void DeviceBleDCSceneContact::InitAttribute(int attributeId, double value)
{
	for (int i = 0; i < 6; i++)
	{
		moduleButton[i]->InitAttribute(attributeId, value);
	}
}

void DeviceBleDCSceneContact::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (data[0] == 0x52)
	{
		if (data[1] == 0x02 && data[2] == 0x00)
		{
			moduleButton[data[3] - 1]->ParseData(&data[4], len - 4, values);
		}
	}
	else if (data[0] == 0x82)
	{
		if (data[1] == 0x24)
		{
			modulePinLevel->ParseData(&data[4], len - 4, values);
		}
	}
	if (values != Json::Value::null)
	{
		PushTelemetry(values);
	}
}

bool DeviceBleDCSceneContact::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	return false;
}

bool DeviceBleDCSceneContact::Do(int id, int value)
{
	LOGD("DoTrigger id: %d, value: %d", id, value);
	return false;
}
