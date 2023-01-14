#include "ElementModeRgb.h"
#include <Log.h>
#include <Util.h>
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementModeRgb::ElementModeRgb(Device *device, uint32_t addr) : Element(device, addr)
{
    mode = 0;
    elementName = "modeRgb";
}

void ElementModeRgb::InitAttribute(int attributeId, double value)
{
    if (attributeId == parameterToId[elementName])
        mode = value;
}

void ElementModeRgb::SaveAttribute()
{
    database->DeviceAttributeAddOrReplace(device, parameterToId[elementName], mode);
}

void ElementModeRgb::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct
    {
        uint16_t idScene;
        uint16_t magic;
        uint8_t mode;
    } data_message_t;
    data_message_t *data_message = NULL;
    if (len == 5 || len == 7)
    {
        data_message = (data_message_t *)(data + 2);
    }
    else
    {
        data_message = (data_message_t *)(data + 2);
    }
    if (data_message->idScene == 0)
    {
        mode = data_message->mode;
        if (1 <= mode && mode <= 6)
        {
            SaveAttribute();
            BuildTelemetryValue(jsonValue);
            CheckTrigger();
        }
    }
}

bool ElementModeRgb::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGD("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isMember("operator") && dataValue["operator"].isString())
    {
        string op = dataValue["operator"].asString();
        if (dataValue.isMember(elementName) && dataValue[elementName].isInt())
        {
            uint16_t mode = dataValue[elementName].asInt();
            rs = Util::CompareNumber(this->mode, mode, op);
            return true;
        }
    }
    return false;
}

void ElementModeRgb::CheckTrigger()
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

void ElementModeRgb::BuildTelemetryValue(Json::Value &jsonValue)
{
    Json::Value dataValue;
    dataValue["ID"] = parameterToId[elementName];
    dataValue["VALUE"] = mode;
    jsonValue.append(dataValue);
}

bool ElementModeRgb::Do(Json::Value &dataValue)
{
    LOGD("DoTrigger data: %s", dataValue.toString().c_str());
    if (dataValue.isMember(elementName) && dataValue[elementName].isInt())
    {
    }
    return false;
}

bool ElementModeRgb::Do(uint16_t value)
{
    LOGD("DoTrigger value: %d", value);
    // bleprotocol call setonoff light
    bleProtocol->CallModeRgb(addr, value);
    return true;
}
