#include "ModuleButtonSeftPowerRemote.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "SceneBle.h"

ModuleButtonSeftPowerRemote::ModuleButtonSeftPowerRemote(Device *device, uint32_t addr) : Module(device, addr)
{
    bt = 0;
    id = 0;
    key = KEY_ATTRIBUTE_BUTTON;
}

ModuleButtonSeftPowerRemote::~ModuleButtonSeftPowerRemote()
{
}

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
        if (data_message->header == RD_OPCODE_SEFTPOWER_REMOTE_PRESS)
        {
            bt = data_message->mode;
            switch (data_message->button)
            {
            case 1:
                id = 126;
                break;
            case 2:
                id = 127;
                break;
            case 3:
                id = 130;
                break;
            case 4:
                id = 128;
                break;
            case 5:
                id = 131;
                break;
            case 6:
                id = 133;
                break;
            case 7:
                break;
            case 8:
                id = 129;
                break;
            case 9:
                id = 132;
                break;
            case 10:
                id = 134;
                break;
            case 11:
                break;
            case 12:
                id = 135;
                break;
            }
            BuildTelemetryValue(jsonValue);
            CheckTrigger();
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
    LOGD("CheckData data: %s", dataValue.toString().c_str());
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
#else
    if (dataValue.isObject() &&
        dataValue.isMember("ID") && dataValue["ID"].isInt())
    {
        int id = dataValue["ID"].asInt();
        if (this->id == id)
        {
            if (dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
                dataValue.isMember("OP") && dataValue["OP"].isString())
            {
                uint16_t bt = 0, mode = 0;
                string op = dataValue["OP"].asString();
                Json::Value listValue = dataValue["VALUE"];
                if (listValue.size() > 0)
                {
                    if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                    {
                        bt = listValue[0].asInt();
                        mode = listValue[1].asInt();
                    }
                    else if (listValue.size() == 1 && listValue[0].isInt())
                    {
                        bt = listValue[0].asInt();
                    }
                    if (this->id == id)
                        rs = Util::CompareNumber(op, this->bt, bt, mode);
                    return true;
                }
            }
        }
    }
#endif
    return false;
}

void ModuleButtonSeftPowerRemote::BuildTelemetryValue(Json::Value &jsonValue)
{
#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
    jsonValue[key] = bt;
#else
    Json::Value dataValue;
    dataValue["ID"] = id;
    dataValue["VALUE"] = bt;
    jsonValue.append(dataValue);
#endif
}
