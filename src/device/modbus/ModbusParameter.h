#pragma once
#include <stdint.h>
#include <string>

using namespace std;

typedef enum
{
	READ_SINGLE_COIL = 1,
	READ_DISCRETE_INPUT,
	READ_MULTI_HOLDING_REGISTER,
	READ_INPUT_REGISTER,
	WRITE_SINGLE_COIL,
	WRITE_SINGLE_HOLDING_REGISTER,
	WRITE_MULTI_COIL = 15,
	WRITE_MULTI_HOLDING_REGISTER
} modbus_function_t;

class ModbusParameter
{
private:
	int id;
	string name;
	int function;
	uint32_t address;
	string dataType;
	bool wordSwap;
	double scale;

public:
	ModbusParameter(int id, string name, int function, uint32_t address, string dataType, bool wordSwap, double scale);

	int GetId();
	string GetName();
	int GetFunction();
	uint32_t GetAddress();
	string GetDataType();
	bool GetWordSwap();
	double GetScale();

	int GetDataLength();
};
