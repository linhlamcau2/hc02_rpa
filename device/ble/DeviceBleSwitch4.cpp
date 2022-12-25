#include "DeviceBleSwitch4.h"
#include <Log.h>

DeviceBleSwitch4::DeviceBleSwitch4(string id, string name, string mac, uint32_t addr)
		: DeviceBle(id, name, mac, addr, BLE_SWITCH_4)
{
	for (int i = 0; i < 4; i++)
	{
		elementOnOff[i] = new ElementOnOff(this, addr + i);
	}
}

bool DeviceBleSwitch4::CheckAddr(uint32_t addr)
{
	return this->addr <= addr && this->addr + 4 > addr;
}

int DeviceBleSwitch4::BuildTelemetryValue(Json::Value &pushDataValue)
{
	for (int i = 0; i < 4; i++)
	{
		elementOnOff[i]->BuildTelemetryValue(pushDataValue);
	}
	return 0;
}

void DeviceBleSwitch4::InitAttribute(int attributeId, double value)
{
	for (int i = 0; i < 4; i++)
	{
		elementOnOff[i]->InitAttribute(attributeId, value);
	}
}

void DeviceBleSwitch4::InputData(uint8_t *data, int len, uint32_t addr)
{
	if (!CheckAddr(addr))
		return;
	typedef struct
	{
		uint16_t u16Opcode;
		uint8_t data[];
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	values = Json::Value::null;
	if (data_message->u16Opcode == 0x0482)
	{
		elementOnOff[addr - this->addr]->ParseData(data_message->data, len - 2, values);
	}
	PushTelemetry(values);
}

bool DeviceBleSwitch4::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	for (int i = 0; i < 4; i++)
	{
		if (elementOnOff[i]->CheckData(dataValue, rs))
		{
			return true;
		}
	}
	return false;
}

bool DeviceBleSwitch4::Do(Json::Value &dataValue)
{
	LOGD("DoTrigger data: %s", dataValue.toString().c_str());
	for (int i = 0; i < 4; i++)
	{
		elementOnOff[i]->Do(dataValue);
	}
	return true;
}

bool DeviceBleSwitch4::Do(int id, int value)
{
	LOGD("DoTrigger id: %d, value: %d", id, value);
	elementOnOff[id - parameterToId["bt0"]]->Do(value);
	return false;
}
