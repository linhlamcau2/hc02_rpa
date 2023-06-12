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
	idPir = BLE_ATTRIBUTE_PIR;
	idLux = BLE_ATTRIBUTE_LUX;
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
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idPir == id || this->idLux == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
			{
				if (this->idPir == id)
					pir = dataValue["VALUE"].asInt();
				else if (this->idLux == id)
					lux = dataValue["VALUE"].asInt();
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return CODE_OK;
			}
		}
	}
#endif
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
		if (lux > 0)
		{
			BuildTelemetryValue(jsonValue);
			CheckTrigger();
			return CODE_OK;
		}
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
	}
	return CODE_ERROR;
}

bool ModulePirLight::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() &&
		dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idPir == id || this->idLux == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
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
	}
#endif
	return false;
}

void ModulePirLight::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_PIR] = pir;
	jsonValue[KEY_ATTRIBUTE_LUX] = lux;
#else
	Json::Value dataValue;
	dataValue["ID"] = idPir;
	dataValue["VALUE"] = pir;
	jsonValue.append(dataValue);
	dataValue["ID"] = idLux;
	dataValue["VALUE"] = lux;
	jsonValue.append(dataValue);
#endif
}
