#include "DeviceBleSwitchScene6ACRgb.h"
#include "Log.h"
#include "Util.h"
#include "Gateway.h"

DeviceBleSwitchScene6ACRgb::DeviceBleSwitchScene6ACRgb(string id, string name, string mac, string data, uint32_t addr, uint32_t type, uint8_t button, uint16_t version)
    : DeviceBle(id, name, mac, data, addr, type, version)
{
#ifdef CONFIG_USE_OLD_APP
    moduleRgb = new ModuleRgb(this, addr, button);
    modules.push_back(moduleRgb);

    for (int i = 0; i < 6; i++)
    {
        moduleButton[i] = new ModuleButton(this, addr, i);
        modules.push_back(moduleButton[i]);
        idButton[i] = Util::GenIdDeviceByElement(id, i+1);
    }
    this->button = button;
#endif
}

#ifdef CONFIG_USE_OLD_APP
int DeviceBleSwitchScene6ACRgb::GetButton()
{
    return this->button;
}
#endif

void DeviceBleSwitchScene6ACRgb::InputData(uint8_t *data, int len, uint32_t addr)
{

	int id = data[3];
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
	deviceData["DEVICE_ID"] = idButton[id];
	deviceData["PROPERTIES"] = values;
	pushDataValue["CMD"] = "DEVICE";
	pushDataValue["DATA"].append(deviceData);
	gateway->PublishToLocalMessage(pushDataValue);
	gateway->PublishToGatewayTelemetry(pushDataValue);
}
