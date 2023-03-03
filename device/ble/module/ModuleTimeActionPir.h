#pragma once
#include "Module.h"

using namespace std;

class ModuleTimeActionPir : public Module
{
protected:
    uint16_t time;
    int id;

public:
    ModuleTimeActionPir(Device *device, uint32_t addr);

#ifdef CONFIG_SAVE_ATTRIBUTE
    void InitAttribute(int attributeId, double value);
    void SaveAttribute();
#endif

    bool InputData(uint8_t *data, int len, Json::Value &jsonValue);

    bool CheckData(Json::Value &dataValue, bool &rs);

    void CheckTrigger();

    void BuildTelemetryValue(Json::Value &jsonValue);

    bool Do(Json::Value &dataValue);
};
