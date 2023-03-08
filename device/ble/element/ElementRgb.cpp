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

bool ElementRgb::InputData(uint8_t *data, int len, Json::Value &jsonValue)
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
		return true;
	}
	return false;
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

bool ElementRgb::Do(Json::Value &dataValue)
{
    LOGD("DoTrigger data: %s", dataValue.toString().c_str());
    if (dataValue.isArray())
    {
        bool isR = false, isG = false, isB = false, isDimOn = false, isDimOff = false;
        uint8_t r, g, b, dimOff, dimOn;
        for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
        {
            Json::Value data = dataValue[i];
            if (data.isObject())
            {
                if (data.isMember("ID") && data["ID"].isInt() && data.isMember("VALUE") && data["VALUE"].isInt())
                {
                    if (data["ID"].asInt() == BLE_ATTRIBUTE_B)
                    {
                        isB = true;
                        b = data["VALUE"].asInt();
                    }
                    else if (data["ID"].asInt() == BLE_ATTRIBUTE_G)
                    {
                        isG = true;
                        g = data["VALUE"].asInt();
                    }
                    else if (data["ID"].asInt() == BLE_ATTRIBUTE_R)
                    {
                        isR = true;
                        r = data["VALUE"].asInt();
                    }
                    else if (data["ID"].asInt() == BLE_ATTRIBUTE_DIM_ON)
                    {
                        isDimOn = true;
                        dimOn = data["VALUE"].asInt();
                    }
                    else if (data["ID"].asInt() == BLE_ATTRIBUTE_DIM_OFF)
                    {
                        isDimOff = true;
                        dimOff = data["VALUE"].asInt();
                    }
                }
            }
        }
        if (isDimOn && isDimOff && isR && isB && isG)
        {
            bleProtocol->ControlRgbSwitch(addr, 0, b, g, r, dimOn, dimOff);
        }
    }
    return false;
}
