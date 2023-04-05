#include "DeviceBle.h"
#include "Log.h"

DeviceBle::DeviceBle(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version) : Device(id, name, mac, data, addr, type, version)
{
	protocol = BLE_DEVICE;
	deviceKey = GetDeviceKey(data);
}

string DeviceBle::GetDeviceKey()
{
	return deviceKey;
}

string DeviceBle::GetDeviceKey(string data)
{
	Json::Value dataJson;
	if (dataJson.parse(data) && dataJson.isObject())
	{
		if (dataJson.isMember("devicekey") && dataJson["devicekey"].isString())
		{
			deviceKey = dataJson["devicekey"].asString();
		}
	}
	return deviceKey;
}
