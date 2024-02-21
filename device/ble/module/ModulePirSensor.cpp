#include "ModulePirSensor.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"
#include "Gateway.h"
#include "SceneBle.h"

ModulePirSensor::ModulePirSensor(Device *device, uint32_t addr) : Module(device, addr)
{
	pir = 0;
	id = BLE_ATTRIBUTE_PIR;
}

ModulePirSensor::~ModulePirSensor()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModulePirSensor::InitAttribute(int id, double value)
{
	if (this->id == id)
		pir = value;
}

void ModulePirSensor::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, id, onoff);
}
#endif

int ModulePirSensor::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isArray())
	{
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			if (dataValue[i].isObject() && dataValue[i].isMember("ID") && dataValue[i]["ID"].isInt())
			{
				int id = dataValue[i]["ID"].asInt();
				if (this->id == id && dataValue[i].isMember("VALUE") && dataValue[i]["VALUE"].isInt())
				{
					pir = dataValue[i]["VALUE"].asInt();
					BuildTelemetryValue(jsonValue);
					return CODE_OK;
				}
			}
		}
	}
	else if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			pir = dataValue["VALUE"].asInt();
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
	}
#endif
	return CODE_ERROR;
}

int ModulePirSensor::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x05 && data[2] == 0x00)
	{
		typedef struct __attribute__((packed))
		{
			uint16_t pir;
			uint16_t scene;
		} data_message_t;
		data_message_t *data_message = (data_message_t *)&data[3];
		pir = (data_message->pir);
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		uint16_t sceneId = data[5] | (data[6] << 8);
		if (sceneId > 0)
		{
			SceneBle *sceneBle = gateway->getSceneBleFromAddr(data_message->scene);
			if (sceneBle)
			{
				for (int i = 0; i < sceneBle->deviceList.size(); i++)
				{
					DeviceBle *dev = (DeviceBle *)sceneBle->deviceList[i]->device;
					if (dev)
					{
						if (sceneBle->deviceList[i]->data.isArray() || sceneBle->deviceList[i]->data.isObject())
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
	return CODE_ERROR;
}

bool ModulePirSensor::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->id == id &&
			dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
			dataValue.isMember("OP") && dataValue["OP"].isString())
		{
			uint16_t value1 = 0, value2 = 0;
			string op = dataValue["OP"].asString();
			Json::Value listValue = dataValue["VALUE"];
			if (listValue.size() > 0)
			{
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					value1 = listValue[0].asInt();
					value2 = listValue[1].asInt();
				}
				else if (listValue.size() == 1 && listValue[0].isInt())
				{
					value1 = listValue[0].asInt();
				}
				rs = Util::CompareNumber(op, this->pir, value1, value2);
				return true;
			}
		}
	}
#endif
	return false;
}

void ModulePirSensor::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_PIR] = pir;
#else
	Json::Value dataValue;
	dataValue["ID"] = id;
	dataValue["VALUE"] = pir;
	jsonValue.append(dataValue);
#endif
}
