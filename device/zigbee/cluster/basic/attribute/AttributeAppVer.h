#pragma once
#include "cluster/Attribute.h"

using namespace std;

class AttributeAppVer : public Attribute
{
private:
	uint8_t appVer;
	string appVerKey;

public:
	AttributeAppVer(Cluster *cluster, string appVerKey);

	int InputData(Json::Value &dataValue, Json::Value &jsonValue);

	/**
	 * @brief Parse raw data to element parameter value
	 *
	 * @param data data from device driver (uart)
	 * @param len length of data
	 * @param jsonValue json value to put parameter after parsing
	 * @return true if data include this element opcode
	 * @return false
	 */
	int InputData(uint8_t *data, int len, Json::Value &jsonValue, int *lenRemain = NULL);

	/**
	 * @brief Build telemetry message with this element
	 *
	 * @param jsonValue
	 */
	void BuildTelemetryValue(Json::Value &jsonValue);
};
