#include "ModuleTimeOffHeatLamp.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleTimeOffHeatLamp::ModuleTimeOffHeatLamp(Device *device, uint32_t addr) : Module(device, addr)
{
    time = 0;
}

ModuleTimeOffHeatLamp::~ModuleTimeOffHeatLamp()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleTimeOffHeatLamp::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_TIMEOFF_HEATLAMP)
        time = value;
}

void ModuleTimeOffHeatLamp::SaveAttribute()
{
    database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_TIMEOFF_HEATLAMP, time);
}
#endif

int ModuleTimeOffHeatLamp::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_TIMEOFF_HEATLAMP) && dataValue[KEY_ATTRIBUTE_TIMEOFF_HEATLAMP].isInt())
    {
        time = dataValue[KEY_ATTRIBUTE_TIMEOFF_HEATLAMP].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleTimeOffHeatLamp::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t vendorId;
        uint16_t header;
        uint16_t time;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_CONFIG_RSP &&
        data_message->vendorId == RD_VENDOR_ID &&
        data_message->header == RD_HEADER_CONFIG_TIMER_OFF_HEAT_LAMP)
    {
        uint16_t time = data_message->time;
        if (this->time != time)
        {
            this->time = time;
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

bool ModuleTimeOffHeatLamp::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_DISTANCE) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[KEY_ATTRIBUTE_DISTANCE].isInt())
        {
            int value = dataValue[KEY_ATTRIBUTE_DISTANCE].asInt();
            rs = Util::CompareNumber(op, this->time, value);
            return true;
        }
        else if (dataValue[KEY_ATTRIBUTE_DISTANCE].isArray())
        {
            Json::Value listValue = dataValue[KEY_ATTRIBUTE_DISTANCE];
            if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
            {
                int value1 = listValue[0].asInt();
                int value2 = listValue[1].asInt();
                rs = Util::CompareNumber(op, this->time, value1, value2);
                return true;
            }
        }
    }
    return false;
}

void ModuleTimeOffHeatLamp::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_TIMEOFF_HEATLAMP] = time;
}

int ModuleTimeOffHeatLamp::Do(Json::Value &dataValue)
{
    LOGV("ModuleTimeOffHeatLamp Do data: %s", dataValue.toString().c_str());
    if (bleProtocol && dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_TIMEOFF_HEATLAMP) && dataValue[KEY_ATTRIBUTE_TIMEOFF_HEATLAMP].isInt())
    {
        uint16_t time = dataValue[KEY_ATTRIBUTE_TIMEOFF_HEATLAMP].asInt();
        if (bleProtocol->SetTimeoffHeatLamp(addr, time) == CODE_OK)
        {
            this->time = time;
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}
