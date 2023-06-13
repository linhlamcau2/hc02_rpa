#include "FunctionFace.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "SceneBle.h"

FunctionFace::FunctionFace(Device *device, string id) : Function(device)
{
	this->id = id;
	face = 0;
}

FunctionFace::~FunctionFace()
{
}

int FunctionFace::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isMember("id") && dataValue["id"].isString() &&
			dataValue.isMember("face") && dataValue["face"].isInt())
	{
		string id = dataValue["id"].asString();
		int face = dataValue["face"].asInt();
		if (this->id == id)
		{
			this->face = face;
			BuildTelemetryValue(jsonValue);
		}
	}
	return CODE_ERROR;
}

bool FunctionFace::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	// TODO:
	return false;
}

void FunctionFace::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_FACE_ID] = id;
	jsonValue[KEY_ATTRIBUTE_FACE_VALUE] = face;
}
