#include "ElementHsl.h"
#include <Log.h>
#include <Util.h>
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementHsl::ElementHsl(Device *device, uint32_t addr) : Element(device, addr)
{
    h = 0;
    l = 0;
    s = 0;
    elementNameH = "hue";
    elementNameL = "luminance";
    elementNameS = "saturation";
}

void ElementHsl::InitAttribute(int attributeId, double value)
{
    if (attributeId == parameterToId[elementNameH])
    {
        h = value;
    }
    else if (attributeId == parameterToId[elementNameS])
    {
        s = value;
    }
    else if (attributeId == parameterToId[elementNameL])
    {
        l = value;
    }

}

void ElementHsl::SaveAttribute()
{
    database->DeviceAttributeAddOrReplace(device, parameterToId[elementNameH], h);
    database->DeviceAttributeAddOrReplace(device, parameterToId[elementNameS], s);
    database->DeviceAttributeAddOrReplace(device, parameterToId[elementNameL], l);
}

void ElementHsl::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct
    {
        uint16_t l;
        uint16_t h;
        uint16_t s;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    l = data_message->l;
    h = data_message->h;
    s = data_message->s;

    SaveAttribute();
    BuildTelemetryValue(jsonValue);
    CheckTrigger();
}

bool ElementHsl::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGD("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isMember("operator") && dataValue["operator"].isString())
    {
        string op = dataValue["operator"].asString();
        if (dataValue.isMember(elementNameH) && dataValue[elementNameH].isInt())
        {
            uint16_t h = dataValue[elementNameH].asInt();
            rs = Util::CompareNumber(this->h, h, op);
            return true;
        }
        if (dataValue.isMember(elementNameS) && dataValue[elementNameS].isInt())
        {
            uint16_t s = dataValue[elementNameS].asInt();
            rs = Util::CompareNumber(this->s, s, op);
            return true;
        }
        if (dataValue.isMember(elementNameL) && dataValue[elementNameL].isInt())
        {
            uint16_t l = dataValue[elementNameL].asInt();
            rs = Util::CompareNumber(this->l, l, op);
            return true;
        }
    }
    return false;
}

void ElementHsl::CheckTrigger()
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

void ElementHsl::BuildTelemetryValue(Json::Value &jsonValue)
{
    Json::Value dataValue;
    dataValue["ID"] = parameterToId[elementNameH];
    dataValue["VALUE"] = h;
    jsonValue.append(dataValue);
    dataValue["ID"] = parameterToId[elementNameS];
    dataValue["VALUE"] = s;
    jsonValue.append(dataValue);
    dataValue["ID"] = parameterToId[elementNameL];
    dataValue["VALUE"] = l;
    jsonValue.append(dataValue);
}

bool ElementHsl::Do(Json::Value &dataValue)
{
    LOGD("DoTrigger data: %s", dataValue.toString().c_str());
    return false;
}

bool ElementHsl::Do(uint16_t valueH, uint16_t valueS, uint16_t valueL)
{
    LOGD("DoTrigger value: %d - %d - %d", valueH, valueS, valueL);
    // bleprotocol call setonoff light
    bleProtocol->SetHSLLight(addr,valueH, valueS, valueL, 0, true);
    return true;
}
