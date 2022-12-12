#include "DeviceModbusES_SM_TH_01.h"

DeviceModbusES_SM_TH_01::DeviceModbusES_SM_TH_01(string id, string name, string mac, uint32_t addr)
		: DeviceModbus(id, name, mac, addr, MODBUS_ES_SM_TH_01)
{
	modbusParameterList.push_back(new ModbusParameter(0, "moisture", READ_MULTI_HOLDING_REGISTER, 0x0000, "u16", false, 0.1));
	modbusParameterList.push_back(new ModbusParameter(0, "temp", READ_MULTI_HOLDING_REGISTER, 0x0001, "u16", false, 0.1));

	modbusParameterList.push_back(new ModbusParameter(0, "write_addr", WRITE_SINGLE_HOLDING_REGISTER, 0x07D0, "i16", false, 1));
	modbusParameterList.push_back(new ModbusParameter(0, "write_baud", WRITE_SINGLE_HOLDING_REGISTER, 0x07D1, "i16", false, 1));
}
