#include "DeviceBleSwitchScene6ACRgb.h"
#include "Log.h"
#include "Util.h"
#include "Gateway.h"

DeviceBleSwitchScene6ACRgb::DeviceBleSwitchScene6ACRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version)
	: DeviceBle(id, name, mac, data, addr, type, version)
{
	for (int i = 0; i < 6; i++)
	{
		moduleRgb[i] = new ModuleRgb(this, addr, i + 1);
		modules.push_back(moduleRgb[i]);
		moduleButton[i] = new ModuleButton(this, addr, i);
		modules.push_back(moduleButton[i]);
		idButton[i] = Util::GenIdDeviceByElement(id, i);
	}
}

bool DeviceBleSwitchScene6ACRgb::CheckId(string id)
{
	for (auto idTemp : idButton)
	{
		if (idTemp == id)
			return true;
	}
	return false;
}

void DeviceBleSwitchScene6ACRgb::InputData(uint8_t *data, int len, uint32_t addr)
{
	int indexButton = data[5];
	if (indexButton >= 1 && indexButton <= 6)
	{
		values = Json::Value::null;
		for (auto &module : modules)
		{
			if (module->InputData(data, len, values) == CODE_OK)
				break;
		}

		Json::Value pushDataValue;
		Json::Value deviceData;
		Json::Value onLine;
		onLine["ID"] = 62;
		onLine["VALUE"] = 1;
		values.append(onLine);
		deviceData["DEVICE_ID"] = idButton[indexButton - 1];
		deviceData["PROPERTIES"] = values;
		pushDataValue["CMD"] = "DEVICE";
		pushDataValue["DATA"].append(deviceData);
		gateway->LocalPublish(pushDataValue);
		gateway->CloudPublish(pushDataValue);
	}
}

int DeviceBleSwitchScene6ACRgb::Do(Json::Value &dataValue, string id)
{
	int index = 0;
	for (int i = 0; i < 6; i++)
	{
		if (idButton[i] == id)
		{
			index = i;
			break;
		}
	}
	if (index >= 0 && index <= 5)
	{
		if (dataValue.isArray())
		{
			for (int j = 0; j < dataValue.size(); j++)
			{
				if (dataValue[j].isObject())
					moduleRgb[index]->Do(dataValue[j]);
			}
		}
		return CODE_OK;
	}

	return CODE_ERROR;
}