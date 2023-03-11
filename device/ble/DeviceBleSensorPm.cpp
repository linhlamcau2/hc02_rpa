#include "DeviceBleSensorPm.h"
#include "Log.h"

DeviceBleSensorPm::DeviceBleSensorPm(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_PM_SENSOR, version)
{
	modulePmSensor = new ModulePmSensor(this, addr);
	powerSource = POWER_BATTERY;
}

int DeviceBleSensorPm::BuildTelemetryValue(Json::Value &pushDataValue)
{
	modulePmSensor->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleSensorPm::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (!modulePmSensor->InputData(data, len, values))
	{
		return;
	}
	PushTelemetry(values);
}

bool DeviceBleSensorPm::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	return false;
}
