#include "DeviceBleOnoffHslModeRGB.h"
#include <Log.h>

DeviceBleOnoffHslModeRGB::DeviceBleOnoffHslModeRGB(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, type, version)
{
	elementOnOff = new ElementOnOff(this, addr);
	elementHsl = new ElementHsl(this, addr);
	elementModeRgb = new ElementModeRgb(this, addr);
}

bool DeviceBleOnoffHslModeRGB::CheckAddr(uint32_t addr)
{
	return ((this->addr <= addr) && (this->addr + 1 >= addr));
}

int DeviceBleOnoffHslModeRGB::BuildTelemetryValue(Json::Value &pushDataValue)
{
	elementOnOff->BuildTelemetryValue(pushDataValue);
	elementHsl->BuildTelemetryValue(pushDataValue);
	elementModeRgb->BuildTelemetryValue(pushDataValue);
	return 0;
}

void DeviceBleOnoffHslModeRGB::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (!elementOnOff->InputData(data, len, values))
	{
		if (!elementHsl->InputData(data, len, values))
		{
			if (!elementModeRgb->InputData(data, len, values))
			{
				return;
			}
		}
	}
	PushTelemetry(values);
}

bool DeviceBleOnoffHslModeRGB::CheckData(Json::Value &dataValue, bool &rs)
{
	LOGD("CheckData data: %s", dataValue.toString().c_str());
	if (!elementOnOff->CheckData(dataValue, rs))
	{
		if (!elementHsl->CheckData(dataValue, rs))
		{
			if (!elementModeRgb->CheckData(dataValue, rs))
			{
				return false;
			}
		}
	}
	return true;
}

bool DeviceBleOnoffHslModeRGB::Do(Json::Value &dataValue)
{
	if (!elementOnOff->Do(dataValue))
	{
		if (!elementHsl->Do(dataValue))
		{
			if (!elementModeRgb->Do(dataValue))
			{
				return false;
			}
		}
	}
	return true;
}

bool DeviceBleOnoffHslModeRGB::AddGroup(uint16_t idGroup, uint16_t epId)
{
	LOGD("AddGroup id: %d epId: %d", idGroup, epId);
	return 0;
}