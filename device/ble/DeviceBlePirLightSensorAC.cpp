#include "DeviceBlePirLightSensorAC.h"
#include "protocol/ble/BleProtocol.h"
#include <Log.h>
#include <util/Util.h>

DeviceBlePirLightSensorAC::DeviceBlePirLightSensorAC(string id, string name, string mac, string device_id, uint32_t addr, uint16_t version)
    : DeviceBle(id, name, mac, device_id, addr, BLE_PIR_LIGHT_SENSOR_AC, version)
{
    modulePirSensor = new ModulePirSensor(this, addr);
    moduleLightSensor = new ModuleLightSensor(this, addr);
    modulePinLevel = new ModulePinLevel(this, addr);
    moduleTimeActionPir = new ModuleTimeActionPir(this, addr);
    powerSource = POWER_BATTERY;
}

int DeviceBlePirLightSensorAC::BuildTelemetryValue(Json::Value &pushDataValue)
{
    modulePirSensor->BuildTelemetryValue(pushDataValue);
    modulePinLevel->BuildTelemetryValue(pushDataValue);
    moduleLightSensor->BuildTelemetryValue(pushDataValue);
    moduleTimeActionPir->BuildTelemetryValue(pushDataValue);
    return 0;
}

void DeviceBlePirLightSensorAC::InputData(uint8_t *data, int len, uint32_t addr)
{
    values = Json::Value::null;
    if (modulePirSensor->InputData(data, len, values))
    {
        PushTelemetry(values);
    }
    if (moduleLightSensor->InputData(data, len, values))
    {
        PushTelemetry(values);
    }
    if (modulePinLevel->InputData(data, len, values))
    {
        PushTelemetry(values);
    }
    if (moduleTimeActionPir->InputData(data, len, values))
    {
        PushTelemetry(values);
    }
}

bool DeviceBlePirLightSensorAC::Do(Json::Value &dataValue)
{
    if (!moduleTimeActionPir->Do(dataValue))
    {
        return false;
    }
    return true;
}
