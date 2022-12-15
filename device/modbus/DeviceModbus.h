#pragma once
#include "Device.h"
#include "ModbusParameter.h"

using namespace std;

class DeviceModbus : public Device
{
protected:
	string serialPort;
	int baudrate;
	int scanRate;
	int timeout;
	time_t readTime;

public:
	Json::Value values;
	vector<ModbusParameter *> modbusParameterList;
	// vector<Json::Value> doDataList;

public:
	DeviceModbus(string id, string name, string mac, uint32_t addr, uint32_t type);

	string GetSerialPort();
	int GetBaudrate();
	int GetScanRate();
	int GetTimeout();
	time_t GetReadTime();
	void SetReadTime(time_t readTime);

	void SetModbusConfig(string serialPort, int baudrate, int scanRate, int timeout);
	void AddParameter(ModbusParameter *modbusParameter);

	int BuildTelemetryValue(Json::Value &pushDataValue);
	bool CheckData(Json::Value &dataValue, bool& rs);
	bool Do(Json::Value &dataValue);
};
