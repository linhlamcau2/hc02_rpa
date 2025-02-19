#include "ModuleStatusLoadHeatLamp.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleStatusLoadHeatLamp::ModuleStatusLoadHeatLamp(Device *device, uint16_t addr) : Module(device, addr)
{
    light = 0;
    fan = 0;
    coolFan = 0;
    heating = 0;
}

ModuleStatusLoadHeatLamp::~ModuleStatusLoadHeatLamp()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleStatusLoadHeatLamp::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_LIGHT_HEATLAMP)
        light = value;
    else if (attribute == KEY_ATTRIBUTE_FAN_HEATLAMP)
        fan = value;
    else if (attribute == KEY_ATTRIBUTE_COOL_FAN_HEATLAMP)
        coolFan = value;
    else if (attribute == KEY_ATTRIBUTE_HEATING_HEATLAMP)
        heating = value;
}

void ModuleStatusLoadHeatLamp::SaveAttribute(string key)
{
    if (key == KEY_ATTRIBUTE_LIGHT_HEATLAMP)
        database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_LIGHT_HEATLAMP, light);
    else if (key == KEY_ATTRIBUTE_FAN_HEATLAMP)
        database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_FAN_HEATLAMP, fan);
    else if (key == KEY_ATTRIBUTE_COOL_FAN_HEATLAMP)
        database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_COOL_FAN_HEATLAMP, coolFan);
    else if (key == KEY_ATTRIBUTE_HEATING_HEATLAMP)
        database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_HEATING_HEATLAMP, heating);
}
#endif

int ModuleStatusLoadHeatLamp::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_LIGHT_HEATLAMP) && dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].isInt() &&
        dataValue.isMember(KEY_ATTRIBUTE_FAN_HEATLAMP) && dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].isInt() &&
        dataValue.isMember(KEY_ATTRIBUTE_COOL_FAN_HEATLAMP) && dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].isInt() &&
        dataValue.isMember(KEY_ATTRIBUTE_HEATING_HEATLAMP) && dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP].isInt())
    {
        light = dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].asInt();
        fan = dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].asInt();
        coolFan = dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].asInt();
        heating = dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP].asInt();
        // CheckTrigger();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleStatusLoadHeatLamp::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t vendorId;
        uint16_t header;
        uint16_t status;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_CONFIG_RSP &&
        data_message->vendorId == RD_VENDOR_ID &&
        data_message->header == RD_HEADER_STATUS_LOAD_HEAT_LAMP)
    {
        uint8_t light = data_message->status & 1;
        uint8_t fan = data_message->status >> 1 & 1;
        uint8_t coolFan = data_message->status >> 2 & 1;
        uint8_t heating1 = data_message->status >> 3 & 1;
        uint8_t heating2 = data_message->status >> 4 & 1;
        uint8_t heating = heating1;
        if (heating2 == 1)
            heating = 2;
        if (light != this->light)
        {
            this->light = light;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute(KEY_ATTRIBUTE_LIGHT_HEATLAMP);
#endif
        }
        if (fan != this->fan)
        {
            this->fan = fan;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute(KEY_ATTRIBUTE_FAN_HEATLAMP);
#endif
        }
        if (coolFan != this->coolFan)
        {
            this->coolFan = coolFan;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute(KEY_ATTRIBUTE_COOL_FAN_HEATLAMP);
#endif
        }
        if (heating != this->heating)
        {
            this->heating = heating;
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute(KEY_ATTRIBUTE_HEATING_HEATLAMP);
#endif
        }

        BuildTelemetryValue(jsonValue);
        CheckTrigger(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

bool ModuleStatusLoadHeatLamp::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue.isMember(KEY_ATTRIBUTE_FAN_HEATLAMP))
        {
            if (dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].isInt())
            {
                int value = dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].asInt();
                rs = Util::CompareNumber(op, this->fan, value);
                return true;
            }
            else if (dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP].isArray())
            {
                Json::Value listValue = dataValue[KEY_ATTRIBUTE_FAN_HEATLAMP];
                if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                {
                    int value1 = listValue[0].asInt();
                    int value2 = listValue[1].asInt();
                    rs = Util::CompareNumber(op, this->fan, value1, value2);
                    return true;
                }
            }
        }
        else if (dataValue.isMember(KEY_ATTRIBUTE_COOL_FAN_HEATLAMP))
        {
            if (dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].isInt())
            {
                int value = dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].asInt();
                rs = Util::CompareNumber(op, this->coolFan, value);
                return true;
            }
            else if (dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP].isArray())
            {
                Json::Value listValue = dataValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP];
                if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                {
                    int value1 = listValue[0].asInt();
                    int value2 = listValue[1].asInt();
                    rs = Util::CompareNumber(op, this->coolFan, value1, value2);
                    return true;
                }
            }
        }
        else if (dataValue.isMember(KEY_ATTRIBUTE_LIGHT_HEATLAMP))
        {
            if (dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].isInt())
            {
                int value = dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].asInt();
                rs = Util::CompareNumber(op, this->light, value);
                return true;
            }
            else if (dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP].isArray())
            {
                Json::Value listValue = dataValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP];
                if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                {
                    int value1 = listValue[0].asInt();
                    int value2 = listValue[1].asInt();
                    rs = Util::CompareNumber(op, this->light, value1, value2);
                    return true;
                }
            }
        }
        else if (dataValue.isMember(KEY_ATTRIBUTE_HEATING_HEATLAMP))
        {
            if (dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP].isInt())
            {
                int value = dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP].asInt();
                rs = Util::CompareNumber(op, this->heating, value);
                return true;
            }
            else if (dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP].isArray())
            {
                Json::Value listValue = dataValue[KEY_ATTRIBUTE_HEATING_HEATLAMP];
                if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                {
                    int value1 = listValue[0].asInt();
                    int value2 = listValue[1].asInt();
                    rs = Util::CompareNumber(op, this->heating, value1, value2);
                    return true;
                }
            }
        }
    }
    return false;
}

void ModuleStatusLoadHeatLamp::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_LIGHT_HEATLAMP] = light;
    jsonValue[KEY_ATTRIBUTE_FAN_HEATLAMP] = fan;
    jsonValue[KEY_ATTRIBUTE_COOL_FAN_HEATLAMP] = coolFan;
    jsonValue[KEY_ATTRIBUTE_HEATING_HEATLAMP] = heating;
}
