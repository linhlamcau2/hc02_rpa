#pragma once
#include "Module.h"

using namespace std;

class ModuleShareTempHum : public Module
{
protected:
	uint16_t temp;
	uint16_t hum;
	int idTemp;
	int idHum;

public:
	ModuleTempHum(Device *device, uint32_t addr);
};
