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
	this->index = index;
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

int ModuleButton::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() && dataValue.isMember(key) && dataValue[key].isInt())
	{
		bt = dataValue[key].asInt();
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

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
		if (data_message->header == REMOTE_MODULE_DC_TYPE || data_message->header == REMOTE_MODULE_AC_TYPE || data_message->header == REMOTE_MUL_RSP_SCENE_ACTIVE)
		{
			if (data_message->btId == index + 1)
			{
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
	}
	return CODE_ERROR;
}

bool ModuleButton::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(key) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[key].isInt())
		{
			int bt = dataValue[key].asInt();
			rs = Util::CompareNumber(op, this->bt, bt);
			return true;
		}
		else if (dataValue[key].isArray())
		{
			Json::Value listValue = dataValue[key];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int bt1 = listValue[0].asInt();
				int bt2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->bt, bt1, bt2);
				return true;
			}
		}
	}
	return false;
}

void ModuleButton::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[key] = bt;
}

int ModuleButton::Do(Json::Value &dataValue)
{
	LOGV("Do data: %s", dataValue.toString().c_str());
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
	return CODE_ERROR;
}
