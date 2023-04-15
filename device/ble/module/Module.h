#pragma once

#include <string>
#include "json.h"
#include "ErrorCode.h"

using namespace std;

class Device;
class Module
{
protected:
	uint32_t addr;
	Device *device;

public:
	Module(Device *device, uint32_t addr);

	/**
	 * @brief Parse raw data to element parameter value
	 *
	 * @param dataValue json data input
	 * @param jsonValue json value to put parameter after parsing
	 * @return true if data include this element opcode
	 * @return false
	 */
	virtual int InputData(Json::Value &dataValue, Json::Value &jsonValue, Json::Value &jsonValueV2) { return CODE_ERROR; }

	/**
	 * @brief Parse raw data to element parameter value
	 *
	 * @param data data from device driver (uart)
	 * @param len length of data
	 * @param jsonValue json value to put parameter after parsing
	 * @return true if data include this element opcode
	 * @return false
	 */
	virtual int InputData(uint8_t *data, int len, Json::Value &jsonValue, Json::Value &jsonValueV2) { return CODE_ERROR; }
	virtual bool CheckData(Json::Value &dataValue, bool &rs) { return false; }

	/**
	 * @brief Build telemetry message with this module
	 *
	 * @param jsonValue
	 */
	virtual void BuildTelemetryValue(Json::Value &jsonValue) {}

	/**
	 * @brief Build telemetry message with this module use message format version 2
	 *
	 * @param jsonValue
	 */
	virtual void BuildTelemetryValueV2(Json::Value &jsonValue) {}

	/**
	 * @brief Do an action
	 *
	 * @param dataValue data of action
	 * @return true
	 * @return false
	 */
	virtual int Do(Json::Value &dataValue) { return CODE_ERROR; }

	/**
	 * @brief Do an action use message format version 2
	 *
	 * @param dataValue data of action
	 * @return true
	 * @return false
	 */
	virtual int DoV2(Json::Value &dataValue) { return CODE_ERROR; }
};
