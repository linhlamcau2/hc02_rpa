#include "ModuleCalibAuto.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleCalibAuto::ModuleCalibAuto(Device *device, uint32_t addr) : Module(device, addr)
{
    time = 0;
}

ModuleCalibAuto::~ModuleCalibAuto()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleCalibAuto::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_CALIB_AUTO)
        time = value;
}

void ModuleCalibAuto::SaveAttribute()
{
    database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_CALIB_AUTO, time);
}
#endif

int ModuleCalibAuto::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_CALIB_AUTO) && dataValue[KEY_ATTRIBUTE_CALIB_AUTO].isInt())
    {
        time = dataValue[KEY_ATTRIBUTE_CALIB_AUTO].asInt();
        // CheckTrigger();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleCalibAuto::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcodeRsp;
        uint16_t vendorId;
        uint16_t header;
        uint16_t time;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcodeRsp == RD_OPCODE_CONFIG_RSP)
    {
        if (data_message->header == RD_OPCODE_CALIBAUTO)
        {
            if (data_message->time != time)
            {
                time = data_message->time;
#ifdef CONFIG_SAVE_ATTRIBUTE
                SaveAttribute();
#endif
            }
            BuildTelemetryValue(jsonValue);
            CheckTrigger(jsonValue);
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}

bool ModuleCalibAuto::CheckData(Json::Value &dataValue, bool &rs)
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

void ModuleCalibAuto::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_CALIB_AUTO] = time;
}

int ModuleCalibAuto::Do(Json::Value &dataValue)
{
    LOGV("ModuleCalibAuto Do data: %s", dataValue.toString().c_str());
    if (bleProtocol && dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_CALIB_AUTO) && dataValue[KEY_ATTRIBUTE_CALIB_AUTO].isInt())
    {
        int time = dataValue[KEY_ATTRIBUTE_CALIB_AUTO].asInt();
        if (bleProtocol->CalibAuto(addr, time) == CODE_OK)
        {
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}
