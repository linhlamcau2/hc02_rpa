#include "DeviceBleSmokeSensor.h"
#include "Log.h"

DeviceBleSmokeSensor::DeviceBleSmokeSensor(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_SMOKE_SENSOR, version)
{
	moduleSmoke = new ModuleSmoke(this, addr);
}

int DeviceBleSmokeSensor::BuildTelemetryValue(Json::Value &pushDataValue)
{
	moduleSmoke->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleSmokeSensor::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (moduleSmoke->InputData(data, len, values))
	{
		PushTelemetry(values);
	}
}
