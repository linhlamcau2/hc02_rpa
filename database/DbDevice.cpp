#include "Db.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"

#define TABLE_NAME "[Device]"

static int DeviceParse(sqlite3_stmt *stmt, void *ptr)
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
				string id = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				uint32_t addr = sqlite3_column_int(stmt, index++);
				uint32_t type = sqlite3_column_int(stmt, index++);
				string firmware_version = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string hardware_version = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				uint32_t active_time = sqlite3_column_int(stmt, index++);
				uint32_t update_time = sqlite3_column_int(stmt, index++);
				string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string devData;
				string decode = macaron::Base64::Decode(data, devData);
				if (decode == "")
				{
					Json::Value devDataJson;
					Json::Reader r;
					r.parse(devData, devDataJson);
					if(devDataJson.isObject())
					{
						uint16_t u16version = (firmware_version[0] - 48) << 8 | (firmware_version[2] - 48);
						gateway->AddNewDevice(id, name, mac, devData, addr, type, u16version, true, false);
					}
				}
			}
			else if (s == SQLITE_DONE)
			{
				return CODE_OK;
			}
			else
			{
				LOGE("DeviceParse");
				return CODE_ERROR;
			}
		}
	}
	return CODE_OK;
}

int Db::DeviceRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceParse);
}

int Db::DeviceAdd(Device *device)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (device_id, name, mac, data, addr, type, firmware_version) VALUES ('" + device->GetId() + "','" + device->GetName() + "','" + device->GetMac() + "','" + macaron::Base64::Encode(device->GetData()) + "'," + to_string(device->GetAddr()) + "," + to_string(device->GetType()) + ",'" + device->GetVersionStr() + "')";
	return Sqlite_Exec(sql);
}

int Db::DeviceUpdate(Device *device)
{
	string sql = "UPDATE " TABLE_NAME " SET device_id='" + device->GetId() + "', name='" + device->GetName() + "', data='" + device->GetData() + "', addr=" + to_string(device->GetAddr()) + ", type=" + to_string(device->GetType()) + " WHERE mac='" + device->GetMac() + "';";
	return Sqlite_Exec(sql);
}

int Db::DeviceDel(Device *device)
{
	return DeviceDel(device->GetMac());
}

int Db::DeviceDel(string mac)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE mac='" + mac + "';";
	return Sqlite_Exec(sql);
}

int Db::DeviceDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
