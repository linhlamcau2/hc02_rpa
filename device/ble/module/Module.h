#pragma once

#include <string>
#include <json.h>

using namespace std;

class Device;
class Module
{
protected:
	Device *device;

public:
	Module(Device *device);
};
