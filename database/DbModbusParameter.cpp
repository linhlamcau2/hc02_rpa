#ifdef CONFIG_ENABLE_MODBUS

#include "Db.h"
#include <Log.h>
#include <Util.h>
#include "DeviceModbus.h"

#define TABLE_NAME "[ModbusParameter]"

static int ModbusParameterParse(sqlite3_stmt *stmt, void *ptr)
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
				int id = sqlite3_column_int(stmt, index++);
				string mac = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				int function = sqlite3_column_int(stmt, index++);
				uint32_t address = sqlite3_column_int(stmt, index++);
				string dataType = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				bool wordSwap = sqlite3_column_int(stmt, index++);
				double scale = sqlite3_column_double(stmt, index++);
				Device *device = gateway->getDevice(mac);
				if (device)
				{
					LOGW("Have device");
					DeviceModbus *deviceModbus = dynamic_cast<DeviceModbus *>(device);
					if (deviceModbus)
					{
						LOGW("Have modbus device");
						ModbusParameter *modbusParameter = new ModbusParameter(id, name, function, address, dataType, wordSwap, scale);
						if (modbusParameter)
						{
							deviceModbus->AddParameter(modbusParameter);
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
				LOGE("ModbusParameterParse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::ModbusParameterRead()
{
	return ReadAll(TABLE_NAME, NULL, ModbusParameterParse);
}

int Db::ModbusParameterAdd(ModbusParameter *modbusParameter, string mac)
{
	string sql = "INSERT INTO " TABLE_NAME " (mac, name, function, address, data_type, word_swap, scale) VALUES (\"" +
							 mac + "\",\"" +
							 modbusParameter->GetName() + "\"," +
							 to_string(modbusParameter->GetFunction()) + "," +
							 to_string(modbusParameter->GetAddress()) + ",\"" +
							 modbusParameter->GetDataType() + "\"," +
							 to_string(modbusParameter->GetWordSwap()) + "," +
							 to_string(modbusParameter->GetScale()) +
							 ");";
	LOGW("ModbusParameterAdd: %s", sql.c_str());
	return Sqlite_Exec(sql);
}

int Db::ModbusParameterUpdate(ModbusParameter *modbusParameter)
{
	string sql = "UPDATE " TABLE_NAME " SET name=\"" + modbusParameter->GetName() +
							 "\",function=" + to_string(modbusParameter->GetFunction()) +
							 ",address=" + to_string(modbusParameter->GetAddress()) +
							 ",data_type=\"" + modbusParameter->GetDataType() +
							 "\",word_swap=" + to_string(modbusParameter->GetWordSwap()) +
							 ",scale=" + to_string(modbusParameter->GetScale()) +
							 " WHERE id=" + to_string(modbusParameter->GetId()) + ";";
	LOGW("ModbusParameterUpdate: %s", sql.c_str());
	return Sqlite_Exec(sql);
}

int Db::ModbusParameterDel(int id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id=" + to_string(id) + ";";
	return Sqlite_Exec(sql);
}

#endif
