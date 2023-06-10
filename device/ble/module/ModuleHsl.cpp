#include "ModuleHsl.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleHsl::ModuleHsl(Device *device, uint32_t addr) : Module(device, addr)
{
	h = 0;
	s = 0;
	l = 0;
	idH = BLE_ATTRIBUTE_HUE;
	idS = BLE_ATTRIBUTE_SATURATION;
	idL = BLE_ATTRIBUTE_LUMINANCE;
	isH = false;
	isS = false;
	isL = false;
}

ModuleHsl::~ModuleHsl()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleHsl::InitAttribute(int id, double value)
{
	if (this->idH == id)
	{
		h = value;
	}
	else if (this->idS == id)
	{
		s = value;
	}
	else if (this->idL == id)
	{
		l = value;
	}
}

void ModuleHsl::SaveAttribute()
{
	database->DeviceAttributeAddOrReplace(device, idH, h);
	database->DeviceAttributeAddOrReplace(device, idS, s);
	database->DeviceAttributeAddOrReplace(device, idL, l);
}
#endif

int ModuleHsl::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idH == id || this->idL == id || this->idS == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
			{
				if (this->idH == id)
					h = dataValue["VALUE"].asInt();
				else if (this->idL == id)
					l = dataValue["VALUE"].asInt();
				else if (this->idS == id)
					s = dataValue["VALUE"].asInt();
				BuildTelemetryValue(jsonValue);
				CheckTrigger();
				return CODE_OK;
			}
		}
	}
#endif
	return CODE_ERROR;
}

int ModuleHsl::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
	{
		uint16_t opcode;
		uint16_t l;
		uint16_t h;
		uint16_t s;
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == BLE_MESH_OPCODE_HSL)
	{
		if ((l != data_message->l) || (h != data_message->h) || (s != data_message->s))
		{
			l = data_message->l;
			h = data_message->h;
			s = data_message->s;
			BuildTelemetryValue(jsonValue);
		}
#ifdef CONFIG_SAVE_ATTRIBUTE
		SaveAttribute();
#endif
		CheckTrigger();
		return CODE_OK;
	}
	return CODE_ERROR;
}

bool ModuleHsl::CheckData(Json::Value &dataValue, bool &rs)
{
	// LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idH == id || this->idS == id || this->idL == id)
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
					if (this->idH == id)
						rs = Util::CompareNumber(op, this->h, value1, value2);
					else if (this->idS == id)
						rs = Util::CompareNumber(op, this->h, value1, value2);
					else if (this->idL == id)
						rs = Util::CompareNumber(op, this->h, value1, value2);
					return true;
				}
			}
		}
	}
#endif
	return false;
}

void ModuleHsl::CheckTrigger()
{
	// LOGV("CheckTrigger");
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		rs = false;
		if (CheckData(*ruleInputDevice->GetData(), rs))
			ruleInputDevice->Trigger(rs);
	}
}

void ModuleHsl::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	jsonValue[KEY_ATTRIBUTE_HUE] = h;
	jsonValue[KEY_ATTRIBUTE_SATURATION] = s;
	jsonValue[KEY_ATTRIBUTE_LUMINANCE] = l;
#else
	Json::Value dataValue;
	dataValue["ID"] = idH;
	dataValue["VALUE"] = h;
	jsonValue.append(dataValue);
	dataValue["ID"] = idS;
	dataValue["VALUE"] = s;
	jsonValue.append(dataValue);
	dataValue["ID"] = idL;
	dataValue["VALUE"] = l;
	jsonValue.append(dataValue);
#endif
}

static uint16_t value_h = 0;
static uint16_t value_s = 0;
static uint16_t value_l = 0;
int ModuleHsl::DoJsonArray(Json::Value &dataValue)
{
	// LOGD("DoJsonArray data: %s", dataValue.toString().c_str());
	if (dataValue.isArray())
	{
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			Json::Value data = dataValue[i];
			if (data.isMember("ID") && data["ID"].isInt() && data.isMember("VALUE") && data["VALUE"].isInt())
			{
				if (data["ID"].asInt() == BLE_ATTRIBUTE_HUE)
				{
					isH = true;
					value_h = data["VALUE"].asInt();
				}
				else if (data["ID"].asInt() == BLE_ATTRIBUTE_SATURATION)
				{
					isS = true;
					value_s = data["VALUE"].asInt();
				}
				else if (data["ID"].asInt() == BLE_ATTRIBUTE_LUMINANCE)
				{
					isL = true;
					value_l = data["VALUE"].asInt();
				}
			}
		}
		if (isL && isS && isH)
		{
			if (bleProtocol)
			{
				bleProtocol->SetHSLLight(addr, value_h, value_s, value_l, 0, true);
			}
			else
				LOGW("BleProtocol null");
		}
	}
	return CODE_ERROR;
}

int ModuleHsl::Do(Json::Value &dataValue)
{
	// LOGD("Module Hsl Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	if (bleProtocol && dataValue.isObject() &&
			dataValue.isMember(KEY_ATTRIBUTE_HUE) && dataValue[KEY_ATTRIBUTE_HUE].isInt() &&
			dataValue.isMember(KEY_ATTRIBUTE_SATURATION) && dataValue[KEY_ATTRIBUTE_SATURATION].isInt() &&
			dataValue.isMember(KEY_ATTRIBUTE_LUMINANCE) && dataValue[KEY_ATTRIBUTE_LUMINANCE].isInt())
	{
		int h = dataValue[KEY_ATTRIBUTE_HUE].asInt();
		int s = dataValue[KEY_ATTRIBUTE_SATURATION].asInt();
		int l = dataValue[KEY_ATTRIBUTE_LUMINANCE].asInt();
		if (bleProtocol->SetHSLLight(addr, h, s, l, 0, true) == CODE_OK)
		{
			this->h = h;
			this->s = s;
			this->l = l;
			return CODE_OK;
		}
	}
#else
	if (dataValue.isObject() &&
			dataValue.isMember("ID") && dataValue["ID"].isInt())
	{
		int id = dataValue["ID"].asInt();
		if (this->idH == id || this->idL == id || this->idS == id)
		{
			if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
			{
				int value = dataValue["VALUE"].asInt();
				if (this->idH == id)
				{
					isH = true;
					value_h = value;
				}
				else if (this->idS == id)
				{
					isS = true;
					value_s = value;
				}
				else if (this->idL == id)
				{
					isL = true;
					value_l = value;
				}
				if (isH && isS && isL)
				{
					isH = false;
					isS = false;
					isL = false;
					if (bleProtocol)
					{
						bleProtocol->SetHSLLight(addr, value_h, value_s, value_l, 0, true);
					}
					else
						LOGW("BleProtocol null");
					return CODE_OK;
				}
			}
		}
	}
#endif
	return CODE_ERROR;
}