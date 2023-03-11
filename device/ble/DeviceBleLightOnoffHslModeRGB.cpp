#include "DeviceBleLightOnoffHslModeRGB.h"
#include "Log.h"

DeviceBleLightOnoffHslModeRGB::DeviceBleLightOnoffHslModeRGB(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr);
	moduleModeRgb = new ModuleModeRgb(this, addr);
	moduleHsl = new ModuleHsl(this, addr);
	powerSource = POWER_AC;
}

bool DeviceBleLightOnoffHslModeRGB::CheckAddr(uint32_t addr)
{
	return ((this->addr <= addr) && (this->addr + 1 >= addr));
}

int DeviceBleLightOnoffHslModeRGB::BuildTelemetryValue(Json::Value &pushDataValue)
{
	moduleOnOff->BuildTelemetryValue(pushDataValue);
	moduleHsl->BuildTelemetryValue(pushDataValue);
	moduleModeRgb->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleLightOnoffHslModeRGB::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (!moduleOnOff->InputData(data, len, values))
	{
		if (!moduleHsl->InputData(data, len, values))
		{
			if (!moduleModeRgb->InputData(data, len, values))
			{
				return;
			}
		}
	}
	PushTelemetry(values);
}

bool DeviceBleLightOnoffHslModeRGB::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (!moduleOnOff->CheckData(dataValue, rs))
	{
		if (!moduleHsl->CheckData(dataValue, rs))
		{
			if (!moduleModeRgb->CheckData(dataValue, rs))
			{
				return false;
			}
		}
	}
	return true;
}

bool DeviceBleLightOnoffHslModeRGB::Do(Json::Value &dataValue)
{
	if (!moduleOnOff->Do(dataValue))
	{
		if (!moduleHsl->Do(dataValue))
		{
			if (!moduleModeRgb->Do(dataValue))
			{
				return false;
			}
		}
	}
	return true;
}

bool DeviceBleLightOnoffHslModeRGB::AddGroup(uint16_t idGroup, uint16_t epId)
{
	LOGD("AddGroup id: %d epId: %d", idGroup, epId);
	return 0;
}
