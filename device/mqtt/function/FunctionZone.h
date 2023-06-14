#pragma once
#include "Function.h"

using namespace std;

class FunctionZone : public Function
{
protected:
	string zoneId;
	int zoneValue;

public:
	FunctionZone(Device *device, string id);
	~FunctionZone();

	/**
	 * @brief Parse raw data to funtion parameter value
	 *
	 * @param dataValue json value input data
	 * @param jsonValue json value to put parameter after parsing
	 * @return true if data include this funtion opcode
	 * @return false
	 */
	int InputData(Json::Value &dataValue, Json::Value &jsonValue);

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
