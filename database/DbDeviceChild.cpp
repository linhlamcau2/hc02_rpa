#include "Db.h"
#include "Log.h"
#include "Util.h"

#define TABLE_NAME "[DeviceChild]"

static int DeviceChildParse(sqlite3_stmt *stmt, void *ptr)
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
				int element = sqlite3_column_int(stmt, index++);
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

int Db::DeviceChildRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceChildParse);
}

int Db::DeviceChildAdd(string deviceId, int element)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (deviceId, element) VALUES (\"" + deviceId + "\"," + to_string(element) + ")";
	return Sqlite_Exec(sql);
}

int Db::DeviceChildUpdate(string deviceId, int element)
{
	string sql = "UPDATE " TABLE_NAME " SET deviceId=\"" + deviceId + "\", element=" + to_string(element) + ";";
	return Sqlite_Exec(sql);
}

int Db::DeviceChildDel(string deviceId)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id = \"" + deviceId + "\";";
	return Sqlite_Exec(sql);
}

int Db::DeviceChildDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
