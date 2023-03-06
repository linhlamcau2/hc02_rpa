#include "DeviceBleLightOnoffCctDimHslModeRGB.h"
#include <Log.h>

DeviceBleLightOnoffCctDimHslModeRGB::DeviceBleLightOnoffCctDimHslModeRGB(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr);
	moduleDim = new ModuleDim(this, addr);
	moduleModeRgb = new ModuleModeRgb(this, addr);
	moduleHsl = new ModuleHsl(this, addr);
	elementCct = new ElementCct(this, addr + 1);
	powerSource = POWER_AC;
}

bool DeviceBleLightOnoffCctDimHslModeRGB::CheckAddr(uint32_t addr)
{
	return ((this->addr <= addr) && (this->addr + 1 >= addr));
}

int DeviceBleLightOnoffCctDimHslModeRGB::BuildTelemetryValue(Json::Value &pushDataValue)
{
	moduleOnOff->BuildTelemetryValue(pushDataValue);
	moduleDim->BuildTelemetryValue(pushDataValue);
	moduleHsl->BuildTelemetryValue(pushDataValue);
	moduleModeRgb->BuildTelemetryValue(pushDataValue);
	elementCct->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleLightOnoffCctDimHslModeRGB::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (!moduleOnOff->InputData(data, len, values))
	{
		if (!moduleDim->InputData(data, len, values))
		{
			if (!moduleHsl->InputData(data, len, values))
			{
				if (!moduleModeRgb->InputData(data, len, values))
				{
					if (!elementCct->InputData(data, len, values))
					{
						return;
					}
				}
			}
		}
	}
	PushTelemetry(values);
}

bool DeviceBleLightOnoffCctDimHslModeRGB::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (!moduleOnOff->CheckData(dataValue, rs))
	{
		if (!moduleDim->CheckData(dataValue, rs))
		{
			if (!moduleHsl->CheckData(dataValue, rs))
			{
				if (!moduleModeRgb->CheckData(dataValue, rs))
				{
					if (!elementCct->CheckData(dataValue, rs))
					{
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool DeviceBleLightOnoffCctDimHslModeRGB::Do(Json::Value &dataValue)
{
	if (!moduleOnOff->Do(dataValue))
	{
		if (!moduleDim->Do(dataValue))
		{
			if (!moduleHsl->Do(dataValue))
			{
				if (!moduleModeRgb->Do(dataValue))
				{
					if (!elementCct->Do(dataValue))
					{
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool DeviceBleLightOnoffCctDimHslModeRGB::AddGroup(uint16_t idGroup, uint16_t epId)
{
	LOGD("AddGroup id: %d epId: %d", idGroup, epId);
	return 0;
}
