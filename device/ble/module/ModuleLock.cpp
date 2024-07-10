#include "ModuleLock.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleLock::ModuleLock(Device *device, uint16_t addr) : Module(device, addr)
{
    lock = 0;
}

ModuleLock::~ModuleLock()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleLock::InitAttribute(string attribute, double value)
{
    if (attribute == KEY_ATTRIBUTE_LOCK)
        lock = value;
}

void ModuleLock::SaveAttribute()
{
    database->DeviceAttributeAdd(device, KEY_ATTRIBUTE_LOCK, lock);
}
#endif

int ModuleLock::InputData(Json::Value &dataValue, Json::Value &jsonValue)
{
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_LOCK) && dataValue[KEY_ATTRIBUTE_LOCK].isInt())
    {
        lock = dataValue[KEY_ATTRIBUTE_LOCK].asInt();
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

int ModuleLock::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    if (data[0] == RD_OPCODE_CONFIG_RSP)
    {
        if (data[3] == 0x11 && data[4] == 0x08)
        {
            if (lock != data[5])
            {
                lock = data[5];
#ifdef CONFIG_SAVE_ATTRIBUTE
                SaveAttribute();
#endif
            }
            BuildTelemetryValue(jsonValue);
            CheckTrigger(jsonValue);
        }
    }

    if (data[0] == 0x52)
    {
        if (data[1] == 0x11 && data[2] == 0x08)
        {
            if (lock != data[3])
            {
                lock = data[3];
#ifdef CONFIG_SAVE_ATTRIBUTE
                SaveAttribute();
#endif
            }
            BuildTelemetryValue(jsonValue);
            CheckTrigger(jsonValue);
        }
    }
    return true;
}

bool ModuleLock::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_LOCK) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[KEY_ATTRIBUTE_LOCK].isInt())
        {
            int value = dataValue[KEY_ATTRIBUTE_LOCK].asInt();
            rs = Util::CompareNumber(op, this->lock, value);
            return true;
        }
        else if (dataValue[KEY_ATTRIBUTE_LOCK].isArray())
        {
            Json::Value listValue = dataValue[KEY_ATTRIBUTE_LOCK];
            if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
            {
                int value1 = listValue[0].asInt();
                int value2 = listValue[1].asInt();
                rs = Util::CompareNumber(op, this->lock, value1, value2);
                return true;
            }
        }
    }
    return false;
}

void ModuleLock::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_LOCK] = lock;
}

int ModuleLock::Do(Json::Value &dataValue)
{
    LOGV("ModuleLock Do data: %s", dataValue.toString().c_str());
    if (bleProtocol && dataValue.isObject() &&
        dataValue.isMember(KEY_ATTRIBUTE_LOCK) && dataValue[KEY_ATTRIBUTE_LOCK].isInt())
    {
        int sttLock = dataValue[KEY_ATTRIBUTE_LOCK].asInt();
        if (bleProtocol->LockDevice(addr, sttLock) == CODE_OK)
        {
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}