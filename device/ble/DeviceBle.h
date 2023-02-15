#pragma once

#include "Device.h"
#include "../../room/Room.h"

using namespace std;

class DeviceBle : public Device
{
protected:
	Json::Value values;

public:
	DeviceBle(string id, string name, string mac, string device_id, uint32_t addr, uint32_t type, uint16_t version);
	int AddDevcieSmartHomeToRoom(Room *room);
};
