#include "DeviceBleOpTran.h"
#include <Log.h>

DeviceBleOpTran::DeviceBleOpTran(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_LED_OP_TRAN, version)
{
	elementOnOff = new ElementOnOff(this, addr);
	elementCct = new ElementCct(this, addr + 1);
	elementDim = new ElementDim(this, addr);
}

int DeviceBleOpTran::BuildTelemetryValue(Json::Value &pushDataValue)
{
	elementOnOff->BuildTelemetryValue(pushDataValue);
	elementCct->BuildTelemetryValue(pushDataValue);
	elementDim->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleOpTran::InputData(uint8_t *data, int len, uint32_t addr)
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
	else if (data_message->u16Opcode == 0x6682)
	{
		elementCct->ParseData(data_message->data, len -2, values);
	}
	else if (data_message->u16Opcode == 0x4e82)
	{
		elementDim->ParseData(data_message->data, len -2, values);
	}
	PushTelemetry(values);
}

bool DeviceBleOpTran::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (elementOnOff->CheckData(dataValue, rs))
	{
		return true;
	}
	return false;
}


bool DeviceBleOpTran::Do(int id, int value)
{
	LOGD("DoTrigger id: %d, value: %d", id, value);
	if (id == 0)
	{
		elementOnOff->Do(value);
	}
	else if (id == 1)
    {
		elementDim->Do((value * 65535) / 100);
	}
	else if (id == 2)
    {
		elementCct->Do((value * 192) + 800);		
	}
	else {
		LOGW("DoTrigger id don't");
	}
	return false;
}

bool DeviceBleOpTran::Do(Json::Value &dataValue)
{
	for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
	{
		Json::Value property = dataValue[i];
		if (property.isMember("ID") && property["ID"].isInt() &&
			property.isMember("VALUE") && property["VALUE"].isInt())
		{
			int id = property["ID"].asInt();
			unsigned int value = property["VALUE"].asInt();
			if (id == 0)
			{
				elementOnOff->Do(value);
			}
			else if (id == 1)
			{
				elementDim->Do((value * 65535) / 100);
			}
			else if (id == 2)
			{
				elementCct->Do((value * 192) + 800);
			}
		}
	}
	return false;
}