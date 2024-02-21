#include "ModuleStatusStartup.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleStatusStartup::ModuleStatusStartup(Device *device, uint32_t addr) : Module(device, addr)
{
    status = 0;
    id = BLE_ATTRIBUTE_STATUS_STARTUP;
}

ModuleStatusStartup::~ModuleStatusStartup()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleStatusStartup::InitAttribute(int id, double value)
{
    if (this->id == id)
        status = value;
}

void ModuleStatusStartup::SaveAttribute()
{
    database->DeviceAttributeAddOrReplace(device, id, status);
}
#endif

int ModuleStatusStartup::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
    if (dataValue.isArray())
    {
        for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
        {
            if (dataValue[i].isObject() && dataValue[i].isMember("ID") && dataValue[i]["ID"].isInt())
            {
                int id = dataValue[i]["ID"].asInt();
                if (this->id == id && dataValue[i].isMember("VALUE") && dataValue[i]["VALUE"].isInt())
                {
                    status = dataValue[i]["VALUE"].asInt();
                    BuildTelemetryValue(jsonValue);
                    return CODE_OK;
                }
            }
        }
    }
    else if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
    {
        int id = dataValue["ID"].asInt();
        if (this->id == id)
            if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
            {
                status = dataValue["VALUE"].asInt();
                BuildTelemetryValue(jsonValue);
                CheckTrigger();
                return CODE_OK;
            }
    }
#endif
    return CODE_ERROR;
}

int ModuleStatusStartup::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    if (data[0] == RD_OPCODE_CONFIG_RSP)
    {
        typedef struct __attribute__((packed))
        {
            uint16_t vendorId;
            uint16_t header;
            uint8_t status;
        } data_message_t;
        data_message_t *data_message = (data_message_t *)&data[1];
        if (data_message->header == RD_OPCODE_CONFIG_STATUS_STARTUP_SWITCH)
        {
            status = data_message->status;
            BuildTelemetryValue(jsonValue);
            CheckTrigger();
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}

bool ModuleStatusStartup::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
    if (dataValue.isObject() &&
        dataValue.isMember("ID") && dataValue["ID"].isInt())
    {
        int id = dataValue["ID"].asInt();
        if (this->id == id &&
            dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
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

                rs = Util::CompareNumber(op, this->status, value1, value2);
                return true;
            }
        }
    }
#endif
    return false;
}

void ModuleStatusStartup::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
    jsonValue[KEY_ATTRIBUTE_STATUS_STARTUP] = status;
#else
    Json::Value dataValue;
    dataValue["ID"] = id;
    dataValue["VALUE"] = status;
    jsonValue.append(dataValue);
#endif
}

int ModuleStatusStartup::Do(Json::Value &dataValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
    if (bleProtocol && dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_STATUS_STARTUP) && dataValue[KEY_ATTRIBUTE_STATUS_STARTUP].isInt())
    {
        int status = dataValue[KEY_ATTRIBUTE_STATUS_STARTUP].asInt();
        if (bleProtocol->ConfigStatusStartupSwitch(addr, status) == CODE_OK)
        {
            this->status = status;
            return CODE_OK;
        }
    }
#else
    if (dataValue.isObject() &&
        dataValue.isMember("ID") && dataValue["ID"].isInt())
    {
        int id = dataValue["ID"].asInt();
        if (this->id == id &&
            dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
        {
            int value = dataValue["VALUE"].asInt();
            if (bleProtocol)
            {
                bleProtocol->ConfigStatusStartupSwitch(addr, value);
            }
            else
                LOGW("BleProtocol null");
            return CODE_OK;
        }
    }
#endif
    return CODE_ERROR;
}