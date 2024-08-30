#include "ModuleOxyWater.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleOxyWater::ModuleOxyWater(Device *device, uint16_t addr) : Module(device, addr)
{
    oxy = 0;
}

ModuleOxyWater::~ModuleOxyWater()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleOxyWater::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_OXY_WATER)
        oxy = value;
}

void ModuleOxyWater::SaveAttribute()
{
    database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_OXY_WATER, oxy);
}
#endif

int ModuleOxyWater::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_OXY_WATER) && dataValue[KEY_ATTRIBUTE_OXY_WATER].isInt())
    {
        oxy = dataValue[KEY_ATTRIBUTE_OXY_WATER].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleOxyWater::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t header;
        uint16_t oxy;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == 0x52 && data_message->header == RD_HEADER_OXY_WATER_STATUS)
    {
        uint16_t tempOxy = data_message->oxy;
        if (oxy != tempOxy)
        {
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute();
#endif
        }
        BuildTelemetryValue(jsonValue);
        CheckTrigger(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

bool ModuleOxyWater::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_OXY_WATER) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[KEY_ATTRIBUTE_OXY_WATER].isInt())
        {
            int oxy = dataValue[KEY_ATTRIBUTE_OXY_WATER].asInt();
            rs = Util::CompareNumber(op, this->oxy, oxy);
            return true;
        }
        else if (dataValue[KEY_ATTRIBUTE_OXY_WATER].isArray())
        {
            Json::Value listValue = dataValue[KEY_ATTRIBUTE_OXY_WATER];
            if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
            {
                int value1 = listValue[0].asInt();
                int value2 = listValue[1].asInt();
                rs = Util::CompareNumber(op, this->oxy, value1, value2);
                return true;
            }
        }
    }
    return false;
}

void ModuleOxyWater::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_OXY_WATER] = oxy;
}
