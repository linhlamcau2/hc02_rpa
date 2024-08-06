#include "ModuleNotifyScene.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Gateway.h"
#include "SceneBle.h"

ModuleNotifyScene::ModuleNotifyScene(Device *device, uint16_t addr) : Module(device, addr)
{
	idScene = 0;
}

ModuleNotifyScene::~ModuleNotifyScene()
{
}

int ModuleNotifyScene::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	idScene = 0;
	typedef struct __attribute__((packed))
	{
		uint8_t opcode;
		uint16_t header;
		uint8_t data[10];
	} data_message_t;
	data_message_t *data_message = (data_message_t *)data;
	if (data_message->opcode == 0x52)
	{
		if (data_message->header == SCREEN_TOUCH_MODULE_TYPE)
		{
			idScene = data_message->data[1] | (data_message->data[2] << 8);
		}
		if (idScene > 0)
		{
			SceneBle *sceneBle = gateway->getSceneBleFromAddr(idScene);
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
	return CODE_ERROR;
}
