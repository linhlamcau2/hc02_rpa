#pragma once

#include <string>
#include "json.h"
#include "Attribute.h"

#define PROFILE_ZHA 0x0104

#define CLUSTER_GENERAL_BASIC 0x0000
#define CLUSTER_ONOFF 0x0006
#define CLUSTER_ILLUMINANCE 0x0400
#define CLUSTER_TEMPERATURE 0x0402
#define CLUSTER_HUMIDITY 0x0405

using namespace std;

int getSizeOfDataType(uint8_t *dataType);

class Device;
class Cluster
{
protected:
	uint16_t id;
	Device *device;
	uint8_t endpoint;
	vector<Attribute *> attributes;

public:
	Cluster(uint16_t id, Device *device, uint8_t endpoint = 1);

	Device *getDevice() { return device; }

	/**
	 * @brief Parse raw data to element parameter value
	 *
	 * @param dataValue json data input
	 * @param jsonValue json value to put parameter after parsing
	 * @return true if data include this element opcode
	 * @return false
	 */
	virtual int InputData(Json::Value &dataValue, Json::Value &jsonValue);

	/**
	 * @brief Parse raw data to element parameter value
	 *
	 * @param data data from device driver (uart)
	 * @param len length of data
	 * @param jsonValue json value to put parameter after parsing
	 * @return true if data include this element opcode
	 * @return false
	 */
	virtual int InputData(uint8_t *data, int len, Json::Value &jsonValue);// { return CODE_ERROR; }

	/**
	 * @brief Check rules related with this element
	 *
	 */
	virtual void CheckTrigger() {}

	virtual bool CheckData(Json::Value &dataValue, bool &rs);

	/**
	 * @brief Build telemetry message with this module
	 *
	 * @param jsonValue
	 */
	virtual void BuildTelemetryValue(Json::Value &jsonValue);

	/**
	 * @brief Do an action
	 *
	 * @param dataValue data of action
	 * @return true
	 * @return false
	 */
	virtual int Do(Json::Value &dataValue);
};
