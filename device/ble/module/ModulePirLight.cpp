#include "ModulePirLight.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"
#include "Gateway.h"
#include "SceneBle.h"

ModulePirLight::ModulePirLight(Device *device, uint32_t addr) : Module(device, addr)
{
	pir = 0;
	lux = 0;
}

ModulePirLight::~ModulePirLight()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModulePirLight::InitAttribute(int id, double value)
{
	if (this->id == idPir)
		pir = value;
	else if (this->id == idLux)
		lux = value;
}

void ModulePirLight::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, idPir, pir);
	database->DeviceAttributeAddOrReplace(device, idLux, lux);
}
#endif

int ModulePirLight::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_PIR) && dataValue[KEY_ATTRIBUTE_PIR].isInt() &&
			dataValue.isMember(KEY_ATTRIBUTE_LUX) && dataValue[KEY_ATTRIBUTE_LUX].isInt())
	{
		pir = dataValue[KEY_ATTRIBUTE_PIR].asInt();
		lux = dataValue[KEY_ATTRIBUTE_LUX].asInt();
		CheckTrigger();
		BuildTelemetryValue(jsonValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int ModulePirLight::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	if (data[0] == 0x52 && data[1] == 0x05 && data[2] == 0x00)
	{
		typedef struct __attribute__((packed))
		{
			uint16_t pir;
			uint16_t scene;
			uint16_t lux;
		} data_message_t;
		data_message_t *data_message = (data_message_t *)&data[3];
		pir = data_message->pir;
		lux = data_message->lux;
		if (lux > 0 && len > 7)
		{
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
	}
	return CODE_ERROR;
}

bool ModulePirLight::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGV("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("op") && dataValue["op"].isString())
	{
		string op = dataValue["op"].asString();
		if (dataValue.isMember(KEY_ATTRIBUTE_PIR))
		{
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
		else if (dataValue.isMember(KEY_ATTRIBUTE_LUX))
		{
			if (dataValue[KEY_ATTRIBUTE_LUX].isInt())
			{
				int lux = dataValue[KEY_ATTRIBUTE_LUX].asInt();
				rs = Util::CompareNumber(op, this->lux, lux);
				return true;
			}
			else if (dataValue[KEY_ATTRIBUTE_LUX].isArray())
			{
				Json::Value listValue = dataValue[KEY_ATTRIBUTE_LUX];
				if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
				{
					int lux1 = listValue[0].asInt();
					int lux2 = listValue[1].asInt();
					rs = Util::CompareNumber(op, this->lux, lux1, lux2);
					return true;
				}
			}
		}
	}
	return false;
}

void ModulePirLight::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_PIR] = pir;
	jsonValue[KEY_ATTRIBUTE_LUX] = lux;
}
