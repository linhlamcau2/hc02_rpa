#include "ModuleCallScene.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "SceneBle.h"

ModuleCallScene::ModuleCallScene(Device *device, uint32_t addr) : Module(device, addr)
{
    idScene = 0;
    id = 0;
    value = 0;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleCallScene::InitAttribute(int id, double value)
{
    if (this->id == id)
        this->value = value;
}

void ModModuleCallSceneuleDim::SaveAttribute()
{
    database->DeviceAttributeAddOrReplace(device, id, value);
}
#endif

// int ModuleCallScene::InputData(Json::Value &dataValue, Json::Value &jsonValue)
// {
//     if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt())
//     {
//         id = dataValue["ID"].asInt();
//         if (dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
//         {
//             value = dataValue["VALUE"].asInt();
//             BuildTelemetryValue(jsonValue);
//             return CODE_OK;
//         }
//     }
//     return CODE_ERROR;
// }

int ModuleCallScene::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct
    {
        uint16_t opcode;
        uint16_t id;
        uint16_t idScene;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == BLE_MESH_OPCODE_RGB)
    {
        if (len == 7 || len == 9)
            idScene = data_message->idScene;
        else
            idScene = data_message->id;
        if (idScene > 0)
        {
            SceneBle *scene = gateway->getSceneBleFromAddr(idScene);
            if (scene)
            {
                for (int i = 0; i < scene->deviceList.size(); i++)
                {
                    if (scene->deviceList[i]->device->GetAddr() == addr)
                    {
                        DeviceBle *dev = (DeviceBle *)scene->deviceList[i]->device;
                        if (dev)
                        {
                            if (scene->deviceList[i]->data.isArray())
                            {
                                for (Json::ArrayIndex j = 0; j < scene->deviceList[i]->data.size(); j++)
                                {
                                    if (scene->deviceList[i]->data[j].isObject())
                                    {
                                        LOGE("Input data json1");
                                        dev->InputData(scene->deviceList[i]->data[j]);
                                    }
                                }
                            }
                            else if (scene->deviceList[i]->data.isObject())
                            {
                                LOGE("Input data json2");
                                dev->InputData(scene->deviceList[i]->data);
                            }
                        }
                        else
                        {
                            LOGW("DeviceBle error");
                        }
                    }
                    else
                    {
                        LOGW("Device not found");
                    }
                }
            }
            else
                LOGW("Scene not found");
        }
        return CODE_OK;
    }
    return CODE_ERROR;
}

bool ModuleCallScene::CheckData(Json::Value &dataValue, bool &rs)
{
    LOGD("CheckData data: %s", dataValue.toString().c_str());
    if (dataValue.isObject() &&
        dataValue.isMember("ID") && dataValue["ID"].isInt())
    {
        int id = dataValue["ID"].asInt();
        if (this->id == id &&
            dataValue.isMember("VALUE") && dataValue["VALUE"].isArray() &&
            dataValue.isMember("OP") && dataValue["OP"].isString())
        {
            uint16_t idScene1 = 0, idScene2 = 0;
            string op = dataValue["OP"].asString();
            Json::Value listValue = dataValue["VALUE"];
            if (listValue.size() > 0)
            {
                if (listValue.size() == 2 && listValue[0].isInt() && listValue[1].isInt())
                {
                    idScene1 = listValue[0].asInt();
                    idScene2 = listValue[1].asInt();
                }
                else if (listValue.size() == 1 && listValue[0].isInt())
                {
                    idScene1 = listValue[0].asInt();
                }
                rs = Util::CompareNumber(this->value, idScene1, idScene2, op);
                return true;
            }
        }
    }
    return false;
}

void ModuleCallScene::CheckTrigger()
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

void ModuleCallScene::BuildTelemetryValue(Json::Value &jsonValue)
{
    Json::Value dataValue;
    dataValue["ID"] = id;
    dataValue["VALUE"] = value;
    jsonValue.append(dataValue);
}
