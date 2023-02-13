#include "DeviceBleSensorTempHum.h"
#include <Log.h>

DeviceBleSensorTempHum::DeviceBleSensorTempHum(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_TEMP_HUM_SENSOR, version)
{
	moduleTempHum = new ModuleTempHum(this, addr);
	modulePinLevel = new ModulePinLevel(this, addr);
}

int DeviceBleSensorTempHum::BuildTelemetryValue(Json::Value &pushDataValue)
{
	moduleTempHum->BuildTelemetryValue(pushDataValue);
	modulePinLevel->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleSensorTempHum::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (!moduleTempHum->InputData(data, len, values))
	{
		if (!modulePinLevel->InputData(data, len, values))
		{
			return;
		}
	}
	PushTelemetry(values);
}

bool DeviceBleSensorTempHum::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	return false;
}
