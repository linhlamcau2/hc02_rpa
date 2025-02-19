#pragma once
#include "Module.h"

using namespace std;

class ModuleControlHeatLamp : public Module
{
protected:
    uint8_t mode;
    uint8_t light;
    uint8_t fan;
    uint8_t coolFan;
    uint8_t heating;

public:
    ModuleControlHeatLamp(Device *device, uint32_t addr);
    ~ModuleControlHeatLamp();

#ifdef CONFIG_SAVE_ATTRIBUTE
    void InitAttribute(string attribute, double value);

    void SaveAttribute(string key);
#endif

    int InputData(Json::Value &dataValue, Json::Value &jsonValue);

    /**
     * @brief Parse raw data to module parameter value
     *
     * @param data data from device driver (uart)
     * @param len length of data
     * @param jsonValue json value to put parameter after parsing
     * @return true if data include this module opcode
     * @return false
     */
    int InputData(uint8_t *data, int len, Json::Value &jsonValue);

    /**
     * @brief Check rule input
     *
     * @param dataValue json rule data input
     * @param rs result of checking
     * @return true if dataValue uses this module paramter
     * @return false if dataValue don't use this module paramter
     */
    bool CheckData(Json::Value &dataValue, bool &rs);

    /**
     * @brief Build telemetry message with this module
     *
     * @param jsonValue
     */
    void BuildTelemetryValue(Json::Value &jsonValue, Json::Value &telemetryMessage);

    /**
     * @brief Do an action
     *
     * @param dataValue data of action
     * @return true
     * @return false
     */
    int Do(Json::Value &dataValue);
};
