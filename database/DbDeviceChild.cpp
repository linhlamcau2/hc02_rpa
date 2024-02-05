#include "Db.h"
#include "Log.h"
#include "Util.h"
#include "DeviceBleSeftPowerRemote.h"

#define TABLE_NAME "[DeviceBleChild]"

static int DeviceBleChildParse(sqlite3_stmt *stmt, void *ptr)
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
				string parentId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));

				Device *child = gateway->getDeviceFromId(deviceId);
				Device *parent = gateway->getDeviceFromId(parentId);
				if (parent)
				{
					if (child)
					{
						DeviceBleSeftPowerRemote *deviceBleSeftPowerRemote = dynamic_cast<DeviceBleSeftPowerRemote *>(child);
						if (deviceBleSeftPowerRemote)
							deviceBleSeftPowerRemote->SetParentDev(parent);
					}
					else
						LOGW("child device not found");
				}
				else
					LOGW("parent device not found");
			}
			else if (s == SQLITE_DONE)
			{
				return CODE_OK;
			}
			else
			{
				LOGE("Device child parese");
				return CODE_ERROR;
			}
		}
	}
	return CODE_OK;
}

int Db::DeviceBleChildRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceBleChildParse);
}

int Db::DeviceBleChildAdd(string deviceId, int element)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (device_id, element) VALUES ('" + deviceId + "'," + to_string(element) + ")";
	return Sqlite_Exec(sql);
}

int Db::DeviceBleChildUpdate(string deviceId, int element)
{
	string sql = "UPDATE " TABLE_NAME " SET device_id='" + deviceId + "', element=" + to_string(element) + ";";
	return Sqlite_Exec(sql);
}

int Db::DeviceBleChildDel(string deviceId)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE device_id = '" + deviceId + "';";
	return Sqlite_Exec(sql);
}

int Db::DeviceBleChildDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
