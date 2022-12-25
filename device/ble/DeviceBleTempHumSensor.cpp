#include "DeviceBleTempHumSensor.h"
#include <Log.h>

DeviceBleTempHumSensor::DeviceBleTempHumSensor(string id, string name, string mac, uint32_t addr)
		: DeviceBle(id, name, mac, addr, BLE_TEMP_HUM_SENSOR)
{
	moduleTempHum = new ModuleTempHum(this);
	modulePinLevel = new ModulePinLevel(this);
}

int DeviceBleTempHumSensor::BuildTelemetryValue(Json::Value &pushDataValue)
{
	moduleTempHum->BuildTelemetryValue(pushDataValue);
	modulePinLevel->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleTempHumSensor::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (data[0] == 0x52)
	{
		if (data[1] == 0x01 && data[2] == 0x00)
		{
			modulePinLevel->ParseData(&data[4], len - 4, values);
		}
		else if (data[1] == 0x06 && data[2] == 0x00)
		{
			moduleTempHum->ParseData(&data[3], len - 3, values);
		}
	}
	if (values != Json::Value::null)
	{
		PushTelemetry(values);
	}
}

bool DeviceBleTempHumSensor::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	return false;
}
