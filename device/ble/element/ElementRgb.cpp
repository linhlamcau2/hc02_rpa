#include "ElementRgb.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementRgb::ElementRgb(Device *device, uint32_t addr) : Element(device, addr)
{
	r = 0;
	b = 0;
	g = 0;
	dimOn = 0;
	dimOff = 0;
	idR = BLE_ATTRIBUTE_R;
	idG = BLE_ATTRIBUTE_G;
	idB = BLE_ATTRIBUTE_B;
	idDimOn = BLE_ATTRIBUTE_DIM_ON;
	idDimOff = BLE_ATTRIBUTE_DIM_OFF;
	isR = isG = isB = isDimOn = isDimOff = false;
	keyR = KEY_ATTRIBUTE_R + to_string(addr - device->GetAddr());
	keyG = KEY_ATTRIBUTE_G + to_string(addr - device->GetAddr());
	keyB = KEY_ATTRIBUTE_B + to_string(addr - device->GetAddr());
	keyDimOn = KEY_ATTRIBUTE_DIM_ON + to_string(addr - device->GetAddr());
	keyDimOff = KEY_ATTRIBUTE_DIM_OFF + to_string(addr - device->GetAddr());
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ElementRgb::InitAttribute(int id, double value)
{
	if (this->id == idR)
	{
		r = value;
	}
	else if (this->id = idG)
	{
		g = value;
	}
	else if (this->id = idB)
	{
		b = value;
	}
	else if (this->id = idDimOn)
	{
		dimOn = value;
	}
	else if (this->id = idDimOff)
	{
		dimOff = value;
	}
}

void ElementRgb::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, idR, r);
	database->DeviceAttributeAddOrReplace(device, idG, g);
	database->DeviceAttributeAddOrReplace(device, idB, b);
	database->DeviceAttributeAddOrReplace(device, idDimOn, dimOn);
	database->DeviceAttributeAddOrReplace(device, idDimOff, dimOff);
}
#endif

bool ElementRgb::InputData(uint8_t *data, int len, Json::Value &jsonValue, Json::Value &jsonValueV2)
{
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t vendorId;
		uint16_t header;
		uint8_t btn;
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t dimOn;
		uint8_t dimOff;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == 0xE3 && data_message->header == 0x050b)
	{
		b = data_message->b;
		g = data_message->g;
		r = data_message->r;
		dimOn = data_message->dimOn;
		dimOff = data_message->dimOff;
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		BuildTelemetryValue(jsonValue);
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ElementRgb::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idR == id || this->idG == id || this->idB == id || this->idDimOn == id || this->idDimOff == id)
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
					if (this->idR == id)
						rs = Util::CompareNumber(this->r, value1, value2, op);
					else if (this->idG == id)
						rs = Util::CompareNumber(this->g, value1, value2, op);
					else if (this->idB == id)
						rs = Util::CompareNumber(this->b, value1, value2, op);
					else if (this->idDimOn == id)
						rs = Util::CompareNumber(this->dimOn, value1, value2, op);
					else if (this->idDimOff == id)
						rs = Util::CompareNumber(this->dimOff, value1, value2, op);
					return true;
				}
			}
		}
	}
	return false;
}

// TODO: can nhac di chuyen den Element.cpp
void ElementRgb::CheckTrigger()
{
	LOGD("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

void ElementRgb::BuildTelemetryValue(Json::Value &jsonValue)
{
	Json::Value dataValue;
	dataValue["ID"] = idR;
	dataValue["VALUE"] = r;
	jsonValue.append(dataValue);
	dataValue["ID"] = idG;
	dataValue["VALUE"] = g;
	jsonValue.append(dataValue);
	dataValue["ID"] = idB;
	dataValue["VALUE"] = b;
	jsonValue.append(dataValue);
	dataValue["ID"] = idDimOn;
	dataValue["VALUE"] = dimOn;
	jsonValue.append(dataValue);
	dataValue["ID"] = idDimOff;
	dataValue["VALUE"] = dimOff;
	jsonValue.append(dataValue);
}

void ElementRgb::BuildTelemetryValueV2(Json::Value &jsonValue)
{
	jsonValue[keyR] = r;
	jsonValue[keyG] = g;
	jsonValue[keyB] = b;
	jsonValue[keyDimOn] = dimOn;
	jsonValue[keyDimOff] = dimOff;
}

// TODO: viet anh recheck DoJsonArray
int ElementRgb::Do(Json::Value &dataValue)
{
	LOGD("Do data: %s", dataValue.toString().c_str());
	if (dataValue.isObject())
	{
		if (dataValue.isMember("ID") && dataValue["ID"].isInt() && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
		{
			if (dataValue["ID"].asInt() == BLE_ATTRIBUTE_B)
			{
				isB = true;
				b = dataValue["VALUE"].asInt();
			}
			else if (dataValue["ID"].asInt() == BLE_ATTRIBUTE_G)
			{
				isG = true;
				g = dataValue["VALUE"].asInt();
			}
			else if (dataValue["ID"].asInt() == BLE_ATTRIBUTE_R)
			{
				isR = true;
				r = dataValue["VALUE"].asInt();
			}
			else if (dataValue["ID"].asInt() == BLE_ATTRIBUTE_DIM_ON)
			{
				isDimOn = true;
				dimOn = dataValue["VALUE"].asInt();
			}
			else if (dataValue["ID"].asInt() == BLE_ATTRIBUTE_DIM_OFF)
			{
				isDimOff = true;
				dimOff = dataValue["VALUE"].asInt();
			}
		}
		if (isDimOn && isDimOff && isR && isB && isG)
		{
			isR = isB = isG = isDimOff = isDimOn = false;
			if (bleProtocol)
				bleProtocol->ControlRgbSwitch(addr, 0, b, g, r, dimOn, dimOff);
			else
				LOGW("BleProtocol null");
			return CODE_OK;
		}
	}
	return CODE_ERROR;
}

int ElementRgb::DoV2(Json::Value &dataValue)
{
	LOGV("DoV2 data: %s", dataValue.toString().c_str());
	if (dataValue.isObject() &&
			dataValue.isMember(keyR) && dataValue[keyR].isInt() &&
			dataValue.isMember(keyG) && dataValue[keyG].isInt() &&
			dataValue.isMember(keyB) && dataValue[keyB].isInt() &&
			dataValue.isMember(keyDimOn) && dataValue[keyDimOn].isInt() &&
			dataValue.isMember(keyDimOff) && dataValue[keyDimOff].isInt())
	{
		int r = dataValue[keyR].asInt();
		int g = dataValue[keyG].asInt();
		int b = dataValue[keyB].asInt();
		int dimOn = dataValue[keyDimOn].asInt();
		int dimOff = dataValue[keyDimOff].asInt();
		if (bleProtocol)
		{
			if (bleProtocol->ControlRgbSwitch(addr, 0, b, g, r, dimOn, dimOff) == CODE_OK)
			{
				this->r = r;
				this->g = g;
				this->b = b;
				this->dimOn = dimOn;
				this->dimOff = dimOff;
			}
		}
		else
			LOGW("BleProtocol null");
		return CODE_OK;
	}
	return CODE_ERROR;
}
