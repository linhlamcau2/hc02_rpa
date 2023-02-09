#include "DeviceBlePmSensor.h"
#include <Log.h>

DeviceBlePmSensor::DeviceBlePmSensor(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_PM_SENSOR, version)
{
	modulePmSensor = new ModulePmSensor(this, addr);
}

int DeviceBlePmSensor::BuildTelemetryValue(Json::Value &pushDataValue)
{
	modulePmSensor->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBlePmSensor::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (!modulePmSensor->InputData(data, len, values))
	{
		return;
	}
	PushTelemetry(values);
}

bool DeviceBlePmSensor::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	return false;
}
