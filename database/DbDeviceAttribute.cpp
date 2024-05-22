#include "Db.h"
#include "Log.h"
#include "Util.h"

#define TABLE_NAME "[DeviceAttribute]"

static int DeviceAttributeParse(sqlite3_stmt *stmt, void *ptr)
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
				string deviceId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string attribute = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				double value = sqlite3_column_double(stmt, index++);
				Device *device = gateway->getDeviceFromId(deviceId);
				if (device)
				{
					device->InitAttribute(attribute, value);
				}
			}
			else if (s == SQLITE_DONE)
			{
				return CODE_OK;
			}
			else
			{
				LOGE("DeviceAttributeParse");
				return CODE_ERROR;
			}
		}
	}
	return CODE_OK;
}

int Db::DeviceAttributeRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceAttributeParse);
}

int Db::DeviceAttributeAdd(Device *device, string attribute, double value)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (device_id, attribute, value) VALUES ('" + device->GetId() + "', '" + attribute + "', " + to_string(value) + ");";
#ifdef __ANDROID__
	pushToListSql(sql);
	return SQLITE_OK;
#else
	return Sqlite_Exec(sql);
#endif
}

int Db::DeviceAttributeUpdate(Device *device, string attribute, double value)
{
	string sql = "UPDATE " TABLE_NAME " SET value=" + to_string(value) + " WHERE device_id='" + device->GetId() + "' AND attribute='" + attribute + "';";
	return Sqlite_Exec(sql);
}

int Db::DeviceAttributeDel(Device *device, string attribute)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE device_id='" + device->GetId() + "' AND attribute='" + attribute + "';";
	return Sqlite_Exec(sql);
}

int Db::DeviceAttributeDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
