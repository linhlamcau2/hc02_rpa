#pragma once
#include "cluster/Attribute.h"

using namespace std;

class AttributeHumidity : public Attribute
{
private:
	uint16_t humidity;
	string humidityKey;

public:
	AttributeHumidity(Cluster *cluster, string humidityKey);

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
	 * @brief Check rule input
	 *
	 * @param dataValue json rule data input
	 * @param rs result of checking
	 * @return true if dataValue uses this element paramter
	 * @return false if dataValue don't use this element paramter
	 */
	bool CheckData(Json::Value &dataValue, bool &rs);

	/**
	 * @brief Build telemetry message with this element
	 *
	 * @param jsonValue
	 */
	void BuildTelemetryValue(Json::Value &jsonValue);
};
