#include "Db.h"
#include <Log.h>
#include <Util.h>

#define TABLE_NAME "[DeviceInGroup]"

static int DeviceInGroupParse(sqlite3_stmt *stmt, void *ptr)
{
	int s, index;
	if (stmt)
	{
		while (1)
		{
			s = sqlite3_step(stmt);
			if (s == SQLITE_ROW)
			{
				index = 1;
				// int id = sqlite3_column_int(stmt, index++);
				int groupId = sqlite3_column_int(stmt, index++);
				string deviceMac = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				int epId = sqlite3_column_int(stmt, index++);
				Group *group = gateway->getGroup(groupId);
				Device *device = gateway->getDevice(deviceMac);
				if (group && device)
				{
					group->AddDevice(device, epId);
				}
			}
			else if (s == SQLITE_DONE)
			{
				return 0;
			}
			else
			{
				LOGE("DeviceInGroupParse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::DeviceInGroupRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceInGroupParse);
}

//TODO: add epId to db
int Db::DeviceInGroupAdd(Group *group, Device *device, int epId)
{
	string sql = "INSERT INTO " TABLE_NAME " (group_id, device_mac) VALUES (" + to_string(group->GetId()) + ",\"" + device->GetMac() + "\")";
	return Sqlite_Exec(sql);
}

int Db::DeviceInGroupDel(Group *group, Device *device, int epId)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE group_id=" + to_string(group->GetId()) + " AND device_mac=\"" + device->GetMac() + "\";";
	return Sqlite_Exec(sql);
}

int Db::DeviceInGroupDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
