#include "Module.h"
#include "Device.h"
#include "Log.h"
#include <thread>

Module::Module(Device *device, uint16_t addr, uint32_t index)
{
	this->device = device;
	this->addr = addr;
	this->index = index;
}

Module::~Module()
{
}

bool Module::CheckAddr(uint16_t addr)
{
	return this->addr == addr;
}

static void CheckInputRuleDevice(void *data, void *jsonValue)
{
	Device *device = (Device *)data;
	Json::Value *dataJsonPtr = (Json::Value *)jsonValue;
	Json::Value &dataJson = *dataJsonPtr;
	bool rs;
	for (auto &ruleInputDevice : device->deviceRuleInputList)
	{
		Json::Value &ruleValue = *(ruleInputDevice->GetData());

		if (dataJson.isObject() && ruleValue.isObject())
		{
			for (auto const &key : dataJson.getMemberNames())
			{
				if (ruleValue.isMember(key))
				{
						rs = false;
						if (device->CheckData(*ruleInputDevice->GetData(), rs))
							ruleInputDevice->Trigger(rs);
				}
			}
		}
		else
		{
			LOGW("Is not object");
		}
	}
}

void Module::CheckTrigger(Json::Value &data)
{
	LOGV("CheckTrigger %s", data.toString().c_str());
	if (!data.isNull())
	{
#ifdef ESP_PLATFORM
		bool rs;
		for (auto &ruleInputDevice : device->deviceRuleInputList)
		{
			Json::Value &ruleValue = *(ruleInputDevice->GetData());
			if (data.isObject() && data.isObject())
			{
				for (auto const &key : data.getMemberNames())
				{
					if (ruleValue.isMember(key))
					{
						if (data[key].type() == ruleValue[key].type())
						{
							rs = false;
							if (CheckData(*ruleInputDevice->GetData(), rs))
								ruleInputDevice->Trigger(rs);
						}
					}
				}
			}
			else
			{
				LOGW("Is not object");
			}
		}
#else
		thread CheckInputRuleDeviceThread(CheckInputRuleDevice, this->device, &data);
		CheckInputRuleDeviceThread.detach();
#endif
	}
}

bool Module::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGW("CheckData not implement");
	return false;
}
