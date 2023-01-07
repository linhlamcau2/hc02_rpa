#include "ElementDim.h"
#include <Log.h>
#include <Util.h>
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementDim::ElementDim(Device *device, uint32_t addr) : Element(device, addr)
{
    dim = 0;
    elementName = "dim";
}

void ElementDim::InitAttribute(int attributeId, double value)
{
    if (attributeId == parameterToId[elementName])
        dim = value;
}

void ElementDim::SaveAttribute()
{
    database->DeviceAttributeAddOrReplace(device, parameterToId[elementName], dim);
}

void ElementDim::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct
    {
        uint16_t dim_first;
        uint16_t dim;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (len <= 3)
        dim = data_message->dim_first;
    else
        dim = data_message->dim;

    SaveAttribute();
    BuildTelemetryValue(jsonValue);
    CheckTrigger();
}

bool ElementDim::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGD("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isMember("operator") && dataValue["operator"].isString())
    {
        string op = dataValue["operator"].asString();
        if (dataValue.isMember(elementName) && dataValue[elementName].isInt())
        {
            uint16_t dim = dataValue[elementName].asInt();
            rs = Util::CompareNumber(this->dim, dim, op);
            return true;
        }
    }
    return false;
}

void ElementDim::CheckTrigger()
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

static int Para2PercentDim(uint16_t para)
{
    return ((para * 100) / 65535);
}
void ElementDim::BuildTelemetryValue(Json::Value &jsonValue)
{
    Json::Value dataValue;
    dataValue["ID"] = parameterToId[elementName];
    dataValue["VALUE"] = Para2PercentDim(dim);
    jsonValue.append(dataValue);
}

bool ElementDim::Do(Json::Value &dataValue)
{
    LOGD("DoTrigger data: %s", dataValue.toString().c_str());
    if (dataValue.isMember(elementName) && dataValue[elementName].isInt())
    {
    }
    return false;
}

bool ElementDim::Do(int value)
{
    LOGD("DoTrigger value: %d", value);
    // bleprotocol call setonoff light
    bleProtocol->SetDimmingLight(addr, value, 0, true);
    return true;
}
