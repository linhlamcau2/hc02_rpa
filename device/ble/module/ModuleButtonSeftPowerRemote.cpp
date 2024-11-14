#include "ModuleButtonSeftPowerRemote.h"
#include "Log.h"
#include "Util.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "SceneBle.h"
#include "Db.h"

ModuleButtonSeftPowerRemote::ModuleButtonSeftPowerRemote(Device *device, uint32_t addr) : Module(device, addr)
{
    bt = 0;
    key = KEY_ATTRIBUTE_BUTTON;
}

ModuleButtonSeftPowerRemote::~ModuleButtonSeftPowerRemote()
{
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleButtonSeftPowerRemote::InitAttribute(string attribute, double value)
{
    if (attribute == key)
        bt = value;
}

void ModuleButtonSeftPowerRemote::SaveAttribute()
{
    database->DeviceAttributeAdd(device, key, bt);
}
#endif

int ModuleButtonSeftPowerRemote::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcodeRsp;
        uint16_t vendorId;
        uint16_t header;
        uint16_t childAddr;
        uint8_t button;
        uint8_t mode;
        uint16_t scene;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcodeRsp == RD_OPCODE_CONFIG_RSP)
    {
        if (data_message->header == RD_HEADER_SEFTPOWER_REMOTE_PRESS)
        {
            if (bt != data_message->mode)
            {
                bt = data_message->mode;
#ifdef CONFIG_SAVE_ATTRIBUTE
                SaveAttribute();
#endif
            }
            switch (data_message->button)
            {
            case 1:
                key = KEY_ATTRIBUTE_BUTTON;
                break;
            case 2:
                key = KEY_ATTRIBUTE_BUTTON "2";
                break;
            case 3:
                key = KEY_ATTRIBUTE_BUTTON;
                BuildTelemetryValue(jsonValue);
                key = KEY_ATTRIBUTE_BUTTON "2";
                break;
            case 4:
                key = KEY_ATTRIBUTE_BUTTON "3";
                break;
            case 5:
                key = KEY_ATTRIBUTE_BUTTON;
                BuildTelemetryValue(jsonValue);
                key = KEY_ATTRIBUTE_BUTTON "3";
                break;
            case 6:
                key = KEY_ATTRIBUTE_BUTTON "2";
                BuildTelemetryValue(jsonValue);
                key = KEY_ATTRIBUTE_BUTTON "3";
                break;
            case 7:
                return CODE_ERROR;
                break;
            case 8:
                key = KEY_ATTRIBUTE_BUTTON "4";
                break;
            case 9:
                key = KEY_ATTRIBUTE_BUTTON;
                BuildTelemetryValue(jsonValue);
                key = KEY_ATTRIBUTE_BUTTON "4";
                break;
            case 10:
                key = KEY_ATTRIBUTE_BUTTON "2";
                BuildTelemetryValue(jsonValue);
                key = KEY_ATTRIBUTE_BUTTON "4";
                break;
            case 11:
                return CODE_ERROR;
                break;
            case 12:
                key = KEY_ATTRIBUTE_BUTTON "3";
                BuildTelemetryValue(jsonValue);
                key = KEY_ATTRIBUTE_BUTTON "4";
                break;
            case 16:
                key = KEY_ATTRIBUTE_BUTTON "5";
                break;
            case 32:
                key = KEY_ATTRIBUTE_BUTTON "6";
                break;
            case 24:
                key = KEY_ATTRIBUTE_BUTTON "4";
                BuildTelemetryValue(jsonValue);
                key = KEY_ATTRIBUTE_BUTTON "5";
                break;
            case 48:
                key = KEY_ATTRIBUTE_BUTTON "5";
                BuildTelemetryValue(jsonValue);
                key = KEY_ATTRIBUTE_BUTTON "6";
                break;
            }
            BuildTelemetryValue(jsonValue);
            CheckTrigger(jsonValue);
            if (data_message->scene > 0)
            {
                SceneBle *sceneBle = gateway->getSceneBleFromAddr(data_message->scene);
                if (sceneBle)
                {
                    for (int i = 0; i < sceneBle->deviceList.size(); i++)
                    {
                        DeviceBle *dev = (DeviceBle *)sceneBle->deviceList[i]->device;
                        if (dev)
                        {
                            if (sceneBle->deviceList[i]->data.isArray())
                            {
                                for (Json::ArrayIndex j = 0; j < sceneBle->deviceList[i]->data.size(); j++)
                                {
                                    if (sceneBle->deviceList[i]->data[j].isObject())
                                    {
                                        dev->InputData(sceneBle->deviceList[i]->data[j]);
                                    }
                                }
                            }
                            else if (sceneBle->deviceList[i]->data.isObject())
                            {
                                dev->InputData(sceneBle->deviceList[i]->data);
                            }
                        }
                        else
                        {
                            LOGW("DeviceBle error");
                        }
                    }
                }
                else
                    LOGW("Scene not found");
            }
            return CODE_OK;
        }
    }
    return CODE_ERROR;
}

bool ModuleButtonSeftPowerRemote::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGV("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember(key) &&
        dataValue.isMember("op") && dataValue["op"].isString())
    {
        string op = dataValue["op"].asString();
        if (dataValue[key].isInt())
        {
            int bt = dataValue[key].asInt();
            rs = Util::CompareNumber(op, this->bt, bt);
            return true;
        }
        else if (dataValue[key].isArray())
        {
            Json::Value listValue = dataValue[key];
            if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
            {
                int bt1 = listValue[0].asInt();
                int bt2 = listValue[1].asInt();
                rs = Util::CompareNumber(op, this->bt, bt1, bt2);
                return true;
            }
        }
    }
    return false;
}

void ModuleButtonSeftPowerRemote::BuildTelemetryValue(Json::Value &jsonValue)
{
    jsonValue[key] = bt;
}
