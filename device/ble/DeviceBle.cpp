#include "DeviceBle.h"

DeviceBle::DeviceBle(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint16_t version) : Device(id, name, mac, data, addr, type, version)
{
	Json::Value dataJson;
	Json::Reader r;
	r.parse(data,dataJson);
	if (dataJson.isObject())
	{
		if (dataJson.isMember("devicekey") && dataJson["devicekey"].isString())
		{
			deviceKey = dataJson["devicekey"].asString();
		}
	}
	protocol = BLE_DEVICE;
}

string DeviceBle::GetDeviceKey()
{
	return deviceKey;
}
