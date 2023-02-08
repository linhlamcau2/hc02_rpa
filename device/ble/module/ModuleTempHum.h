#pragma once
#include "Module.h"

using namespace std;

class ModuleTempHum : public Module
{
protected:
	int temp;
	int hum;
	int idTemp;
	int idHum;

public:
	ModuleTempHum(Device *device);

	int GetTemp();
	void SetTemp(int temp);
	int GetHum();
	void SetHum(int hum);

	bool InputData(uint8_t *data, int len, Json::Value &jsonValue);
	bool CheckData(Json::Value &dataValue, bool &rs);
	void CheckTrigger();
	void BuildTelemetryValue(Json::Value &jsonValue);
};
