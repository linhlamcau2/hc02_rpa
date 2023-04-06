#include "ModuleNotifyScene.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "SceneBle.h"

ModuleNotifyScene::ModuleNotifyScene(Device *device, uint32_t addr) : Module(device, addr)
{
    idScene = 0;
}

int ModuleNotifyScene::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint8_t opcode;
        uint16_t header;
        uint8_t button;
        uint16_t idSceneScreenTouch;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == 0x52)
    {
        if (data_message->header == SCREEN_TOUCH_MODULE_TYPE)
        {
            idScene = data_message->idSceneScreenTouch;
        }
        if (idScene > 0)
        {
            SceneBle *scene = gateway->getSceneBleFromAddr(idScene);
            if (scene)
            {
                for (int i = 0; i < scene->deviceList.size(); i++)
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
                                    dev->InputData(scene->deviceList[i]->data[j]);
                                }
                            }
                        }
                        else if (scene->deviceList[i]->data.isObject())
                        {
                            dev->InputData(scene->deviceList[i]->data);
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
    return CODE_ERROR;
}
