#include "ModbusParameter.h"

ModbusParameter::ModbusParameter(int id, string name, int function, uint32_t address, string dataType, bool wordSwap, double scale)
{
	this->id = id;
	this->name = name;
	this->function = function;
	this->address = address;
	this->dataType = dataType;
	this->wordSwap = wordSwap;
	this->scale = scale;
}

int ModbusParameter::GetId()
{
	return id;
}

string ModbusParameter::GetName()
{
	return name;
}

int ModbusParameter::GetFunction()
{
	return function;
}

uint32_t ModbusParameter::GetAddress()
{
	return address;
}

string ModbusParameter::GetDataType()
{
	return dataType;
}

bool ModbusParameter::GetWordSwap()
{
	return wordSwap;
}

double ModbusParameter::GetScale()
{
	return scale;
}


int ModbusParameter::GetDataLength()
{
	if (dataType == "u16" || dataType == "i16" || dataType == "f16")
		return 1;
	if (dataType == "u32" || dataType == "i32" || dataType == "f32")
		return 2;
	if (dataType == "u48" || dataType == "i48" || dataType == "f48")
		return 3;
	if (dataType == "u64" || dataType == "i64" || dataType == "f64")
		return 4;
	return -1;
}