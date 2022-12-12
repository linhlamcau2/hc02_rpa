#ifdef CONFIG_ENABLE_MODBUS

#include "Db.h"
#include <Log.h>
#include <Util.h>
#include "DeviceModbus.h"
#include "ModbusProtocol.h"

#define TABLE_NAME "[ModbusDevice]"

static int ModbusDeviceParse(sqlite3_stmt *stmt, void *ptr)
{
	int s, index;
	if (stmt)
	{
		while (1)
		{
			s = sqlite3_step(stmt);
			if (s == SQLITE_ROW)
			{
				index = 0;
				string mac = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string serialPort = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				int baudrate = sqlite3_column_int(stmt, index++);
				int scanRate = sqlite3_column_int(stmt, index++);
				int timeout = sqlite3_column_int(stmt, index++);
				Device *device = gateway->getDevice(mac);
				if (device)
				{
					DeviceModbus *deviceModbus = dynamic_cast<DeviceModbus *>(device);
					if (deviceModbus)
					{
						deviceModbus->SetModbusConfig(serialPort, baudrate, scanRate, timeout);
						if (modbusProtocol)
						{
							modbusProtocol->AddDeviceModbus(deviceModbus);
						}
						else
						{
							LOGW("Must init Modbus Protocol first");
						}
					}
					else
					{
						LOGW("Cast modbus device false");
					}
				}
				else
				{
					LOGW("Not found device");
				}
			}
			else if (s == SQLITE_DONE)
			{
				return 0;
			}
			else
			{
				LOGE("ModbusDeviceParse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::ModbusDeviceRead()
{
	return ReadAll(TABLE_NAME, NULL, ModbusDeviceParse);
}

int Db::ModbusDeviceAdd(string mac, string serialPort, int baudrate, int modbusAddress, int scanRate, int timeout)
{
	string sql = "INSERT INTO " TABLE_NAME " (mac, serial_port, baudrate, modbus_address, scan_rate, timeout) VALUES (\"" +
							 mac + "\",\"" +
							 serialPort + "\"," +
							 to_string(baudrate) + "," +
							 to_string(modbusAddress) + "," +
							 to_string(scanRate) + "," +
							 to_string(timeout) +
							 ");";
	LOGW("ModbusDeviceAdd: %s", sql.c_str());
	return Sqlite_Exec(sql);
}

int Db::ModbusDeviceUpdate(string mac, string serialPort, int baudrate, int modbusAddress, int scanRate, int timeout)
{
	string sql = "UPDATE " TABLE_NAME " SET serial_port=\"" + serialPort +
							 "\",baudrate=" + to_string(baudrate) +
							 ",modbus_address=" + to_string(modbusAddress) +
							 ",scan_rate=" + to_string(scanRate) +
							 ",timeout=" + to_string(timeout) +
							 " WHERE mac=\"" + mac + "\";";
	LOGW("ModbusDeviceUpdate: %s", sql.c_str());
	return Sqlite_Exec(sql);
}

int Db::ModbusDeviceDel(string mac)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE mac=\"" + mac + "\";";
	return Sqlite_Exec(sql);
}

int Db::ModbusDeviceDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}

#endif
