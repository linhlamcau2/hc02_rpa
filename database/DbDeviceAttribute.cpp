#include "Db.h"
#include <Log.h>
#include <Util.h>

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
				string deviceMac = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				int attributeId = sqlite3_column_int(stmt, index++);
				double value = sqlite3_column_double(stmt, index++);
				Device *device = gateway->getDevice(deviceMac);
				if (device)
				{
					device->InitAttribute(attributeId, value);
				}
			}
			else if (s == SQLITE_DONE)
			{
				return 0;
			}
			else
			{
				LOGE("DeviceAttributeParse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::DeviceAttributeRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceAttributeParse);
}

int Db::DeviceAttributeAdd(Device *device, int attributeId, double value)
{
	string sql = "INSERT INTO " TABLE_NAME " (mac, attribute_id, value) VALUES (\"" + device->GetMac() + "\", " + to_string(attributeId) + ", " + to_string(value) + ")";
	return Sqlite_Exec(sql);
}

int Db::DeviceAttributeUpdate(Device *device, int attributeId, double value)
{
	string sql = "UPDATE " TABLE_NAME " SET value=" + to_string(value) + " WHERE mac=\"" + device->GetMac() + "\" AND attribute_id=" + to_string(attributeId) + ";";
	return Sqlite_Exec(sql);
}

int Db::DeviceAttributeAddOrReplace(Device *device, int attributeId, double value)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (mac, attribute_id, value) VALUES (\"" + device->GetMac() + "\", " + to_string(attributeId) + ", " + to_string(value) + ")";
	return Sqlite_Exec(sql);
}

int Db::DeviceAttributeDel(Device *device, int attributeId)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE mac=\"" + device->GetMac() + "\" AND attribute_id=" + to_string(attributeId) + ";";
	return Sqlite_Exec(sql);
}

int Db::DeviceAttributeDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
