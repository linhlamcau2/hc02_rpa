#include "DeviceBleDoorSensor.h"
#include "Log.h"

DeviceBleDoorSensor::DeviceBleDoorSensor(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_DOOR_SENSOR, version)
{
	moduleDoorHangOn = new ModuleDoorHangOn(this, addr);
	moduleDoorStatus = new ModuleDoorStatus(this, addr);
	modulePinLevel = new ModulePinLevel(this, addr);
	powerSource = POWER_BATTERY;
}

int DeviceBleDoorSensor::BuildTelemetryValue(Json::Value &pushDataValue)
{
	moduleDoorHangOn->BuildTelemetryValue(pushDataValue);
	modulePinLevel->BuildTelemetryValue(pushDataValue);
	moduleDoorStatus->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleDoorSensor::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (!moduleDoorHangOn->InputData(data, len, values))
	{
		if (!moduleDoorStatus->InputData(data, len, values))
		{
			if (!modulePinLevel->InputData(data, len, values))
			{
				return;
			}
		}
	}
	PushTelemetry(values);
}
