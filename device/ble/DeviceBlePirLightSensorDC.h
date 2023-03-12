#pragma once

#include "DeviceBle.h"
#include "module/ModulePirSensor.h"
#include "module/ModuleLightSensor.h"
#include "module/ModulePinLevel.h"
#include "module/ModuleTimeActionPir.h"
#include <mutex>

using namespace std;

class DeviceBlePirLightSensorDC : public DeviceBle
{
private:
	ModulePirSensor *modulePirSensor;
	ModuleLightSensor *moduleLightSensor;
	ModulePinLevel *modulePinLevel;
	ModuleTimeActionPir *moduleTimeActionPir;

	typedef struct __attribute__((packed))
	{
		string id;
		Json::Value dataValue;
	} item_buf_t;

	vector<item_buf_t> bufConfig;
	vector<item_buf_t> bufConfigV2;
	mutex mtx;

	bool CheckBufConfig();
	bool Config(item_buf_t &data);
	bool DelItemBuf(string id);
	bool PushToBuf(Json::Value data);
	bool PushToBufV2(Json::Value data);

public:
	DeviceBlePirLightSensorDC(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);

	int BuildTelemetryValue(Json::Value &pushDataValue);
	void InputData(uint8_t *data, int len, uint32_t addr = 0);
	bool Do(Json::Value &dataValue);
	bool DoV2(Json::Value &dataValue);
};
