#include "DeviceBlePirLightSensorDC.h"
#include "BleProtocol.h"
#include "Log.h"
#include "Util.h"

DeviceBlePirLightSensorDC::DeviceBlePirLightSensorDC(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
		: DeviceBle(id, name, mac, device_id, addr, BLE_PIR_LIGHT_SENSOR_DC, version)
{
	modulePirSensor = new ModulePirSensor(this, addr);
	moduleLightSensor = new ModuleLightSensor(this, addr);
	modulePinLevel = new ModulePinLevel(this, addr);
	moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
	powerSource = POWER_BATTERY;
}

int DeviceBlePirLightSensorDC::BuildTelemetryValue(Json::Value &pushDataValue)
{
	modulePirSensor->BuildTelemetryValue(pushDataValue);
	modulePinLevel->BuildTelemetryValue(pushDataValue);
	moduleLightSensor->BuildTelemetryValue(pushDataValue);
	moduleTimeActionPir->BuildTelemetryValue(pushDataValue);
	return 0;
}

int DeviceBlePirLightSensorDC::BuildTelemetryValueV2(Json::Value &pushDataValue)
{
	modulePirSensor->BuildTelemetryValueV2(pushDataValue);
	modulePinLevel->BuildTelemetryValueV2(pushDataValue);
	moduleLightSensor->BuildTelemetryValueV2(pushDataValue);
	moduleTimeActionPir->BuildTelemetryValueV2(pushDataValue);
	return 0;
}

// messgae config

/*
scene pir = 1
{
		"pir":1,
		"scene":1,
		"lux":[200,400]
}

delscene
{
		"sceneDel" : 3
}

time action
{
		"time": 30 <don vi giay>
}
*/

bool DeviceBlePirLightSensorDC::Config(item_buf_t &data)
{
	Json::Value dataValue = data.dataValue;
	string id = data.id;
	if (dataValue.isMember("pir") && dataValue["pir"].isInt() && dataValue.isMember("scene") && dataValue["scene"].isInt() && dataValue.isMember("lux") && dataValue["lux"].isArray())
	{
		int pir = dataValue["pir"].asInt();
		uint16_t scene = dataValue["scene"].asInt();
		uint16_t luxLow = dataValue["lux"][0].asInt() / 10;
		uint16_t luxHigh = dataValue["lux"][1].asInt() / 10;

		if (bleProtocol->SetScenePirLightSensor(addr, 2, pir, luxLow, luxHigh, scene, 1) == 0)
		{
			DelItemBuf(id);
		}
	}
	if (dataValue.isMember("time") && dataValue["time"].isInt())
	{
		if (moduleTimeActionPir->Do(dataValue))
		{
			DelItemBuf(id);
		}
	}
	if (dataValue.isMember("sceneDel") && dataValue["sceneDel"].isInt())
	{
		if (bleProtocol->DelScenePirLightSensor(addr, dataValue["sceneDel"].asInt()) == 0)
		{
			DelItemBuf(id);
		}
	}
	return false;
}

void DeviceBlePirLightSensorDC::InputData(uint8_t *data, int len, uint32_t addr)
{
	values = Json::Value::null;
	if (modulePirSensor->InputData(data, len, values))
	{
		PushTelemetry(values);
		CheckBufConfig();
	}
	if (moduleLightSensor->InputData(data, len, values))
	{
		PushTelemetry(values);
		CheckBufConfig();
	}
	if (modulePinLevel->InputData(data, len, values))
	{
		PushTelemetry(values);
		CheckBufConfig();
	}
	if (moduleTimeActionPir->InputData(data, len, values))
	{
		PushTelemetry(values);
		CheckBufConfig();
	}
}

bool DeviceBlePirLightSensorDC::CheckBufConfig()
{
	mtx.lock();
	if (bufConfig.size() > 0)
	{
		Config(*bufConfig.begin());
	}
	mtx.unlock();
	return true;
}

bool DeviceBlePirLightSensorDC::PushToBuf(Json::Value data)
{
	string id = Util::genRandRQI(16);
	item_buf_t item_buf = {id, data};

	mtx.lock();
	bufConfig.push_back(item_buf);
	mtx.unlock();

	Config(item_buf);
	return false;
}

bool DeviceBlePirLightSensorDC::PushToBufV2(Json::Value data)
{
	string id = Util::genRandRQI(16);
	item_buf_t item_buf = {id, data};

	mtx.lock();
	bufConfigV2.push_back(item_buf);
	mtx.unlock();

	Config(item_buf);
	return false;
}

bool DeviceBlePirLightSensorDC::DelItemBuf(string id)
{
	for (auto i = bufConfig.begin(); i != bufConfig.end(); ++i)
	{
		if (i->id == id)
		{
			bufConfig.erase(i);
			bufConfig.shrink_to_fit();
		}
	}
	return true;
}

bool DeviceBlePirLightSensorDC::Do(Json::Value &dataValue)
{
	PushToBuf(dataValue);
	return false;
}

// TODO: need to handle data in bufConfigV2
bool DeviceBlePirLightSensorDC::DoV2(Json::Value &dataValue)
{
	PushToBufV2(dataValue);
	return false;
}
