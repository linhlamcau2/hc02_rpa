#include "ModuleDim.h"
#include <byteswap.h>
#include <Log.h>
#include <Util.h>
#include "Device.h"

ModuleDim::ModuleDim(Device *device) : Module(device)
{
	dim = 0;
}

void ModuleDim::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct
	{
		uint16_t dim;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	dim = bswap_16(data_message->dim);
	BuildTelemetryValue(jsonValue);
	LOGW("dim: %d", dim);
	CheckTrigger();
}

bool ModuleDim::CheckData(Json::Value dataValue)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	bool rs = false;
	if (dataValue.isMember("operator") && dataValue["operator"].isString())
	{
		string op = dataValue["operator"].asString();
		if (dataValue.isMember("dim") && dataValue["dim"].isInt())
		{
			int dim = dataValue["dim"].asInt();
			rs = Util::CompareNumber(this->dim, dim, op);
		}
	}
	return rs;
}

void ModuleDim::CheckTrigger()
{
	LOGD("CheckTrigger");
	for (auto &sceneInputDevice : device->deviceSceneInputList)
	{
		if (CheckData(*sceneInputDevice->GetData()))
			sceneInputDevice->Trigger(true);
	}
}

void ModuleDim::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = parameterToId["dim"];
	dataValue["VALUE"] = dim;
	jsonValue.append(dataValue);
}
