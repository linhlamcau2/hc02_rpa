#include "ModuleButton.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "SceneBle.h"

ModuleButton::ModuleButton(Device *device, uint32_t addr) : ModuleButton(device, addr, 0)
{
}

ModuleButton::ModuleButton(Device *device, uint32_t addr, int index) : Module(device, addr)
{
	bt = 0;
	id = BLE_ATTRIBUTE_BUTTON_1 + addr - device->GetAddr() + index;
	key = KEY_ATTRIBUTE_BUTTON + to_string(addr - device->GetAddr() + index);
}

ModuleButton::~ModuleButton()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleButton::InitAttribute(int id, double value)
{
	if (this->id == id)
		bt = value;
}

void ModuleButton::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, bt);
}
#endif

// int ModuleButton::InputData(Json::Value &dataValue, Json::Value &jsonValue)
// {
// 	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
// 	{
// 		int id = dataValue["ID"].asInt();
// 		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
// 		{
// 			bt = dataValue["VALUE"].asInt();
// 			BuildTelemetryValue(jsonValue);
// 			CheckTrigger();
// 			return CODE_OK;
// 		}
// 	}
// 	return CODE_ERROR;
// }

int ModuleButton::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t header;
		uint8_t btId;
		uint8_t mode;
		uint16_t scene;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == 0x52)
	{
		if (data_message->header == REMOTE_MODULE_DC_TYPE || data_message->header == REMOTE_MODULE_AC_TYPE)
		{
			id = 10 + data_message->btId;
			bt = data_message->mode;
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			if (data_message->scene > 0)
			{
				SceneBle *sceneBle = gateway->getSceneBleFromAddr(data_message->scene);
				if (sceneBle)
				{
					for (int i = 0; i < sceneBle->deviceList.size(); i++)
					{
						DeviceBle *dev = (DeviceBle *)sceneBle->deviceList[i]->device;
						if (dev)
						{
							if (sceneBle->deviceList[i]->data.isArray())
							{
								for (Json::ArrayIndex j = 0; j < sceneBle->deviceList[i]->data.size(); j++)
								{
									if (sceneBle->deviceList[i]->data[j].isObject())
									{
										dev->InputData(sceneBle->deviceList[i]->data[j]);
									}
								}
							}
							else if (sceneBle->deviceList[i]->data.isObject())
							{
								dev->InputData(sceneBle->deviceList[i]->data);
							}
						}
						else
						{
							LOGW("DeviceBle error");
						}
					}
				}
				else
					LOGW("Scene not found");
			}
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

bool ModuleButton::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
					dataValue.isMember("OP") && dataValue["OP"].isString())
			{
				uint16_t bt = 0, mode = 0;
				string op = dataValue["OP"].asString();
				Json::Value listValue = dataValue["VALUE"];
				if (listValue.size() > 0)
				{
					if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
					{
						bt = listValue[0].asInt();
						mode = listValue[1].asInt();
					}
					else if (listValue.size() == 1 && listValue[0].isInt())
					{
						bt = listValue[0].asInt();
					}
					if (this->id == id)
						rs = Util::CompareNumber(this->bt, bt, mode, op);
					return true;
				}
			}
		}
	}
#endif
	return false;
}

// TODO: can nhac di chuyen den Module.cpp
void ModuleButton::CheckTrigger()
{
	LOGV("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

void ModuleButton::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[key] = bt;
#else
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = bt;
	jsonValue.append(dataValue);
#endif
}

int ModuleButton::Do(Json::Value &dataValue)
{
	// LOGD("ModuleButton Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(key) && dataValue[key].isInt())
	{
		int bt = dataValue[key].asInt();
		if (bleProtocol->SetOnOffLight(addr, bt, 0, true) == CODE_OK)
		{
			this->bt = bt;
			return CODE_OK;
		}
	}
#else
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			int value = dataValue["VALUE"].asInt();
			if (bleProtocol)
				bleProtocol->SetOnOffLight(addr, value, 0, true);
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
#endif
	else
	{
		LOGW("Message format error");
	}
	return CODE_ERROR;
}
