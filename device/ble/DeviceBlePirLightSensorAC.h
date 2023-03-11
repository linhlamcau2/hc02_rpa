#pragma once

#include "DeviceBle.h"
#include "module/ModulePirSensor.h"
#include "module/ModuleLightSensor.h"
#include "module/ModulePinLevel.h"
#include "module/ModuleTimeActionPir.h"

using namespace std;

class DeviceBlePirLightSensorAC : public DeviceBle
{
private:
    ModulePirSensor *modulePirSensor;
    ModuleLightSensor *moduleLightSensor;
    ModulePinLevel *modulePinLevel;
    ModuleTimeActionPir *moduleTimeActionPir;

public:
    DeviceBlePirLightSensorAC(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version);

    int BuildTelemetryValue(Json::Value &pushDataValue);
    void InputData(uint8_t *data, int len, uint32_t addr = 0);
    bool Do(Json::Value &dataValue);
};
