#pragma once

#include <string>
#include <json.h>

using namespace std;

class Device;
class Element
{
protected:
	uint32_t addr;
	Device *device;

public:
	Element(Device *device, uint32_t addr);

	/**
	 * @brief Parse raw data to element parameter value
	 *
	 * @param data data from device driver (uart)
	 * @param len length of data
	 * @param jsonValue json value to put parameter after parsing
	 * @return true if data include this element opcode
	 * @return false
	 */
	virtual bool InputData(uint8_t *data, int len, Json::Value &jsonValue) { return false; }

	// /**
	//  * @brief Build telemetry message with this module
	//  *
	//  * @param jsonValue
	//  */
	// virtual void BuildTelemetryValue(Json::Value &jsonValue) {}

	// /**
	//  * @brief Build telemetry message with this module use message format version 2
	//  *
	//  * @param jsonValue
	//  */
	// virtual void BuildTelemetryValueV2(Json::Value &jsonValue) {}

	// /**
	//  * @brief Do an action
	//  *
	//  * @param dataValue data of action
	//  * @return true
	//  * @return false
	//  */
	// virtual bool Do(Json::Value &dataValue) { return false; }

	// /**
	//  * @brief Do an action use message format version 2
	//  *
	//  * @param dataValue data of action
	//  * @return true
	//  * @return false
	//  */
	// virtual bool DoV2(Json::Value &dataValue) { return false; }
};
