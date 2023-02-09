#pragma once
#include "Module.h"

using namespace std;

class ModuleDim : public Module
{
protected:
	uint16_t dim;
	int id;

public:
	ModuleDim(Device *device);

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

	/**
	 * @brief Parse raw data to element parameter value
	 *
	 * @param data data from device driver (uart)
	 * @param len length of data
	 * @param jsonValue json value to put parameter after parsing
	 * @return true if data include this element opcode
	 * @return false
	 */
	bool InputData(uint8_t *data, int len, Json::Value &jsonValue);

	/**
	 * @brief Check rule input
	 *
	 * @param dataValue json rule data input
	 * @param rs result of checking
	 * @return true if dataValue uses this element paramter
	 * @return false if dataValue don't use this element paramter
	 */
	bool CheckData(Json::Value &dataValue, bool &rs);

	/**
	 * @brief Check rules related with this element
	 *
	 */
	void CheckTrigger();

	/**
	 * @brief Build telemetry message with this element
	 *
	 * @param jsonValue
	 */
	void BuildTelemetryValue(Json::Value &jsonValue);
};
