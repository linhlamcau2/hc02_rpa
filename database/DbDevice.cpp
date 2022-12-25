#include "Db.h"
#include <Log.h>
#include <Util.h>

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
				gateway->AddNewDevice(id, name, mac, addr, type, true, false);
			}
			else if (s == SQLITE_DONE)
			{
				return 0;
			}
			else
			{
				LOGE("DeviceParse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::DeviceRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceParse);
}

int Db::DeviceAdd(Device *device)
{
	string sql = "INSERT INTO " TABLE_NAME " (id, name, mac, addr, type) VALUES (\"" + device->GetId() + "\",\"" + device->GetName() + "\",\"" + device->GetMac() + "\"," + to_string(device->GetAddr()) + "," + to_string(device->GetType()) + ")";
	return Sqlite_Exec(sql);
}

int Db::DeviceUpdate(Device *device)
{
	string sql = "UPDATE " TABLE_NAME " SET id=\"" + device->GetId() + "\", name=\"" + device->GetName() + "\", addr=" + to_string(device->GetAddr()) + ", type=" + to_string(device->GetType()) + " WHERE mac=\"" + device->GetMac() + "\";";
	return Sqlite_Exec(sql);
}

int Db::DeviceDel(Device *device)
{
	return DeviceDel(device->GetMac());
}

int Db::DeviceDel(string mac)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE mac=\"" + mac + "\";";
	return Sqlite_Exec(sql);
}

int Db::DeviceDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
