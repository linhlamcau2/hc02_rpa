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
}

ModuleCallScene::~ModuleCallScene()
{
}

int ModuleCallScene::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
	typedef struct __attribute__((packed))
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
			SceneBle *sceneBle = gateway->getSceneBleFromAddr(idScene);
			if (sceneBle)
			{
				for (int i = 0; i < sceneBle->deviceList.size(); i++)
				{
					if (sceneBle->deviceList[i]->device->GetAddr() == addr)
					{
						DeviceBle *dev = (DeviceBle *)sceneBle->deviceList[i]->device;
						if (dev)
						{
							if (sceneBle->deviceList[i]->data.isArray() || sceneBle->deviceList[i]->data.isObject())
							{
								dev->InputData(sceneBle->deviceList[i]->data);
							}
						}
						else
						{
							LOGW("DeviceBle error");
						}
						break;
					}
					else
					{
						// LOGW("Device not found");
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
