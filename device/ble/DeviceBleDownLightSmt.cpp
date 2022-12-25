#include "DeviceBleDownLightSmt.h"
#include <Log.h>

DeviceBleDownLightSmt::DeviceBleDownLightSmt(string id, string name, string mac, uint32_t addr)
		: DeviceBle(id, name, mac, addr, BLE_DOWNLIGHT_SMT)
{
	elementOnOff = new ElementOnOff(this, addr);
}

int DeviceBleDownLightSmt::BuildTelemetryValue(Json::Value &pushDataValue)
{
	elementOnOff->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleDownLightSmt::InputData(uint8_t *data, int len, uint32_t addr)
{
	typedef struct
	{
		uint16_t u16Opcode;
		uint8_t data[];
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	values = Json::Value::null;
	if (data_message->u16Opcode == 0x0482)
	{
		elementOnOff->ParseData(data_message->data, len - 2, values);
	}
	PushTelemetry(values);
}

bool DeviceBleDownLightSmt::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (elementOnOff->CheckData(dataValue, rs))
	{
		return true;
	}
	return false;
}

bool DeviceBleDownLightSmt::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	elementOnOff->Do(dataValue);
	return true;
}

bool DeviceBleDownLightSmt::Do(int id, int value)
{
	LOGD("DoTrigger id: %d, value: %d", id, value);
	return false;
}
