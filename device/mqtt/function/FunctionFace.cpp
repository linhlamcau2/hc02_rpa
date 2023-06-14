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
	if (dataValue.isMember(KEY_ATTRIBUTE_FACE_ID) && dataValue[KEY_ATTRIBUTE_FACE_ID].isString() &&
			dataValue.isMember(KEY_ATTRIBUTE_FACE_VALUE) && dataValue[KEY_ATTRIBUTE_FACE_VALUE].isInt())
	{
		string faceId = dataValue[KEY_ATTRIBUTE_FACE_ID].asString();
		int faceValue = dataValue[KEY_ATTRIBUTE_FACE_VALUE].asInt();
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
	if (dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_FACE_ID) && dataValue[KEY_ATTRIBUTE_FACE_ID].isString())
	{
		string faceId = dataValue[KEY_ATTRIBUTE_FACE_ID].asString();
		if (faceId == this->faceId)
		{
			if (dataValue.isMember("op") && dataValue["op"].isString() &&
					dataValue.isMember(KEY_ATTRIBUTE_FACE_VALUE) && dataValue[KEY_ATTRIBUTE_FACE_VALUE].isInt())
			{
				string op = dataValue["op"].asString();
				int value = dataValue[KEY_ATTRIBUTE_FACE_VALUE].asInt();
				rs = Util::CompareNumber(op, this->faceValue, value);
				return true;
			}
		}
	}
	return false;
}

void FunctionFace::BuildTelemetryValue(Json::Value &jsonValue)
{
	jsonValue[KEY_ATTRIBUTE_FACE_ID] = faceId;
	jsonValue[KEY_ATTRIBUTE_FACE_VALUE] = faceValue;
}
