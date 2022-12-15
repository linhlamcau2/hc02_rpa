#ifdef CONFIG_ENABLE_MODBUS

#pragma once

#include <stdint.h>
#include <vector>
#include <thread>
#include <mutex>
#include <modbus/modbus.h>
#include "DeviceModbus.h"

using namespace std;

class ModbusProtocol
{
private:
	string name;
	string device;		 // ip
	uint32_t baudrate; // port
	string config;

	thread *modbusThread;

public:
	string type;
	modbus_t *ctx;
	mutex mtx;
	vector<DeviceModbus *> deviceModbusList;

	ModbusProtocol(string name, string type, string device, uint32_t baudrate, string config);
	virtual ~ModbusProtocol();

	int Start();
	int Stop();
	int init();
	void AddDeviceModbus(DeviceModbus *deviceModbus);
	void RemoveDeviceModbus(DeviceModbus *deviceModbus);

	bool DoDeviceTrigger(DeviceModbus *deviceModbus, Json::Value dataValue);
	// void DoDeviceTrigger();
};

extern ModbusProtocol *modbusProtocol;

#endif
