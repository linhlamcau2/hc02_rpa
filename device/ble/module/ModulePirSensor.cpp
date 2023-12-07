#include "ModulePirSensor.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"
#include "Gateway.h"
#include "SceneBle.h"

ModulePirSensor::ModulePirSensor(Device *device, uint16_t addr) : Module(device, addr)
{
	pir = 0;
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
	database->DeviceAttributeAddOrReplace(device, id, pir);
}
#endif

int ModulePirSensor::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_PIR) && dataValue[KEY_ATTRIBUTE_PIR].isInt())
	{
		pir = dataValue[KEY_ATTRIBUTE_PIR].asInt();
		// CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
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
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
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
	return CODE_ERROR;
}

bool ModulePirSensor::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_PIR) &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue[KEY_ATTRIBUTE_PIR].isInt())
		{
			int pir = dataValue[KEY_ATTRIBUTE_PIR].asInt();
			rs = Util::CompareNumber(op, this->pir, pir);
			return true;
		}
		else if (dataValue[KEY_ATTRIBUTE_PIR].isArray())
		{
			Json::Value listValue = dataValue[KEY_ATTRIBUTE_PIR];
			if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
			{
				int pir1 = listValue[0].asInt();
				int pir2 = listValue[1].asInt();
				rs = Util::CompareNumber(op, this->pir, pir1, pir2);
				return true;
			}
		}
	}
	return false;
}

void ModulePirSensor::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_PIR] = pir;
}
