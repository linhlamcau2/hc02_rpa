#include "DeviceBleLightOnoffCctDim.h"
#include "Log.h"

DeviceBleLightOnoffCctDim::DeviceBleLightOnoffCctDim(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, type, version)
{
	moduleOnOff = new ModuleOnOff(this, addr);
	moduleDim = new ModuleDim(this, addr);
	elementCct = new ElementCct(this, addr + 1);
	powerSource = POWER_AC;
}

bool DeviceBleLightOnoffCctDim::CheckAddr(uint32_t addr)
{
	return ((this->addr <= addr) && (this->addr + 1 >= addr));
}

int DeviceBleLightOnoffCctDim::BuildTelemetryValue(Json::Value &pushDataValue)
{
	moduleOnOff->BuildTelemetryValue(pushDataValue);
	moduleDim->BuildTelemetryValue(pushDataValue);
	elementCct->BuildTelemetryValue(pushDataValue);
	return 0;
}

int DeviceBleLightOnoffCctDim::BuildTelemetryValueV2(Json::Value &pushDataValue)
{
	moduleOnOff->BuildTelemetryValueV2(pushDataValue);
	moduleDim->BuildTelemetryValueV2(pushDataValue);
	elementCct->BuildTelemetryValueV2(pushDataValue);
	return 0;
}

void DeviceBleLightOnoffCctDim::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (!moduleOnOff->InputData(data, len, values))
	{
		if (!moduleDim->InputData(data, len, values))
		{
			if (!elementCct->InputData(data, len, values))
			{
				return;
			}
		}
	}
	PushTelemetry(values);
}

bool DeviceBleLightOnoffCctDim::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (!moduleOnOff->CheckData(dataValue, rs))
	{
		if (!moduleDim->CheckData(dataValue, rs))
		{
			if (!elementCct->CheckData(dataValue, rs))
			{
				return false;
			}
		}
	}
	return true;
}

bool DeviceBleLightOnoffCctDim::Do(Json::Value &dataValue)
{
	if (!moduleOnOff->Do(dataValue))
	{
		if (!moduleDim->Do(dataValue))
		{
			if (!elementCct->Do(dataValue))
			{
				return false;
			}
		}
	}
	return true;
}

bool DeviceBleLightOnoffCctDim::DoV2(Json::Value &dataValue)
{
	moduleOnOff->DoV2(dataValue);
	moduleDim->DoV2(dataValue);
	elementCct->DoV2(dataValue);
	return true;
}
