#include "ModuleDistance.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleDistance::ModuleDistance(Device *device, uint32_t addr) : Module(device, addr)
{
    distance = 0;
    id = BLE_ATTRIBUTE_DISTANCE;
}

ModuleDistance::~ModuleDistance()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleDistance::InitAttribute(int id, double value)
{
    if (this->id == id)
        distance = value;
}

void ModuleDistance::SaveAttribute()
{
    database->DeviceAttributeAddOrReplace(device, id, distance);
}
#endif

int ModuleDistance::InputData(Json::Value &dataValue, Json::Value &jsonValue)
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
                    distance = dataValue[i]["VALUE"].asInt();
                    BuildTelemetryValue(jsonValue);
                    return CODE_OK;
                }
            }
        }
    }
    else if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
    {
        int id = dataValue["ID"].asInt();
        if (this->id == id && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
        {
            distance = dataValue["VALUE"].asInt();
            BuildTelemetryValue(jsonValue);
            // CheckTrigger();
            return CODE_OK;
        }
    }
#endif
    return CODE_ERROR;
}

int ModuleDistance::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t vendorId;
        uint16_t header;
        uint8_t distance;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == RD_OPCODE_CONFIG_RSP)
    {
        if (data_message->header == RD_OPCODE_CONFIG_SET_DISTANCE_RADA_SENSOR)
        {
            distance = data_message->distance;
            BuildTelemetryValue(jsonValue);
#ifdef CONFIG_SAVE_ATTRIBUTE
            SaveAttribute();
#endif
            CheckTrigger();
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}

bool ModuleDistance::CheckData(Json::Value &dataValue, bool &rs)
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
            uint16_t dis1 = 0, dis2 = 0;
            string op = dataValue["OP"].asString();
            Json::Value listValue = dataValue["VALUE"];
            if (listValue.size() > 0)
            {
                if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                {
                    dis1 = listValue[0].asInt();
                    dis2 = listValue[1].asInt();
                }
                else if (listValue.size() == 1 && listValue[0].isInt())
                {
                    dis1 = listValue[0].asInt();
                }
                rs = Util::CompareNumber(op, this->distance, dis1, dis2);
                return true;
            }
        }
    }
#endif
    return false;
}

void ModuleDistance::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
    jsonValue[KEY_ATTRIBUTE_DISTANCE] = distance;
#else
    Json::Value dataValue;
    dataValue["ID"] = id;
    dataValue["VALUE"] = distance;
    jsonValue.append(dataValue);
#endif
}

int ModuleDistance::Do(Json::Value &dataValue)
{
    // LOGD("ModuleDim Do data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
    if (bleProtocol && dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_DISTANCE) && dataValue[KEY_ATTRIBUTE_DISTANCE].isInt())
    {
        int value = dataValue[KEY_ATTRIBUTE_DISTANCE].asInt();
        if (bleProtocol->SetDistanceSensor(addr, value) == CODE_OK)
        {
            this->distance = value;
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
                bleProtocol->SetDistanceSensor(addr, value);
            }
            else
                LOGW("BleProtocol null");
            return CODE_OK;
        }
    }
#endif
    return CODE_ERROR;
}
