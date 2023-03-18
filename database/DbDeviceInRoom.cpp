#include "Db.h"
#include "Log.h"
#include "Util.h"

#define TABLE_NAME "[DeviceInRoom]"

static int DeviceInRoomParse(sqlite3_stmt *stmt, void *ptr)
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
				string roomId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string deviceId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				Room *room = gateway->getRoomFromId(roomId);
				Device *device = gateway->getDeviceFromId(deviceId);
				if (room && device)
				{
					room->AddDevice(device, false);
				}
			}
			else if (s == SQLITE_DONE)
			{
				return 0;
			}
			else
			{
				LOGE("DeviceInRoomParse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::DeviceInRoomRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceInRoomParse);
}

int Db::DeviceInRoomAdd(Room *room, Device *device)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (room_id, device_id) VALUES (\"" + room->GetId()+ "\",\"" + device->GetId() + "\");";
	return Sqlite_Exec(sql);
}

int Db::DeviceInRoomDel(Room *room, Device *device)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE room_id= \"" + room->GetId() + "\" AND device_id=\"" + device->GetId() + "\";";
	return Sqlite_Exec(sql);
}

int Db::DeviceInRoomDelAll(Room *room)
{
	string sql = "DELETE FROM " TABLE_NAME " ; ";
	return Sqlite_Exec(sql);
}
