#include "FunctionFace.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "SceneBle.h"

FunctionFace::FunctionFace(Device *device, string faceId) : Function(device)
{
	this->faceId = faceId;
	faceValue = 0;
}

FunctionFace::~FunctionFace()
{
}

int FunctionFace::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
	if (dataValue.isMember("faceId") && dataValue["faceId"].isString() &&
			dataValue.isMember("faceValue") && dataValue["faceValue"].isInt())
	{
		string faceId = dataValue["faceId"].asString();
		int faceValue = dataValue["faceValue"].asInt();
		if (this->faceId == faceId)
		{
			this->faceValue = faceValue;
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
	jsonValue[KEY_ATTRIBUTE_FACE_ID] = faceId;
	jsonValue[KEY_ATTRIBUTE_FACE_VALUE] = faceValue;
}
