#include "DeviceBleBulb.h"
#include <Log.h>

DeviceBleBulb::DeviceBleBulb(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_LED_BULB, version)
{
	elementOnOff = new ElementOnOff(this, addr);
	elementCct = new ElementCct(this, addr + 1);
	elementDim = new ElementDim(this, addr);
	elementHsl = new ElementHsl(this, addr);
	elementModeRgb = new ElementModeRgb(this, addr);
}

bool DeviceBleBulb::CheckAddr(uint32_t addr)
{
	return ((this->addr <= addr) && (this->addr + 1 >= addr));
}

int DeviceBleBulb::BuildTelemetryValue(Json::Value &pushDataValue)
{
	elementOnOff->BuildTelemetryValue(pushDataValue);
	elementCct->BuildTelemetryValue(pushDataValue);
	elementDim->BuildTelemetryValue(pushDataValue);
	elementHsl->BuildTelemetryValue(pushDataValue);
	elementModeRgb->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleBulb::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (!elementOnOff->InputData(data, len, values))
	{
		if (!elementCct->InputData(data, len, values))
		{
			if (!elementDim->InputData(data, len, values))
			{
				if (!elementHsl->InputData(data, len, values))
				{
					if (!elementModeRgb->InputData(data, len, values))
					{
						return;
					}
				}
			}
		}
	}
	PushTelemetry(values);
}

bool DeviceBleBulb::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (!elementOnOff->CheckData(dataValue, rs))
	{
		if (!elementCct->CheckData(dataValue, rs))
		{
			if (!elementDim->CheckData(dataValue, rs))
			{
				if (!elementHsl->CheckData(dataValue, rs))
				{
					if (!elementModeRgb->CheckData(dataValue, rs))
					{
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool DeviceBleBulb::Do(Json::Value &dataValue)
{
	if (!elementOnOff->Do(dataValue))
	{
		if (!elementCct->Do(dataValue))
		{
			if (!elementDim->Do(dataValue))
			{
				if (!elementHsl->Do(dataValue))
				{
					if (!elementModeRgb->Do(dataValue))
					{
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool DeviceBleBulb::AddGroup(uint16_t idGroup, uint16_t epId)
{
	LOGD("AddGroup id: %d epId: %d", idGroup, epId);
	return 0;
}