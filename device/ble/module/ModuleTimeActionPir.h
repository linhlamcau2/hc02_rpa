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
	~ModuleTimeActionPir();

#ifdef CONFIG_SAVE_ATTRIBUTE
	/**
	 * @brief Init parameter value from database after system start
	 *
	 * @param attributeId id of attribute
	 * @param value value of attribute
	 */
	void InitAttribute(int attributeId, double value);

	/**
	 * @brief Save parameter value to database
	 *
	 */
	void SaveAttribute();
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
	 * @brief Check rules related with this module
	 *
	 */
	void CheckTrigger();

	/**
	 * @brief Build telemetry message with this module
	 *
	 * @param jsonValue
	 */
	void BuildTelemetryValue(Json::Value &jsonValue);

	/**
	 * @brief Do an action
	 *
	 * @param dataValue data of action
	 * @return true
	 * @return false
	 */
	int Do(Json::Value &dataValue);

#ifdef CONFIG_USE_MESSAGE_FORMAT_V2
	/**
	 * @brief Build telemetry message with this module
	 *
	 * @param jsonValue
	 */
	void BuildTelemetryValueV2(Json::Value &jsonValue);

	/**
	 * @brief Do an action use message format version 2
	 *
	 * @param dataValue data of action
	 * @return true
	 * @return false
	 */
	int DoV2(Json::Value &dataValue);
#endif // CONFIG_USE_MESSAGE_FORMAT_V2
};
