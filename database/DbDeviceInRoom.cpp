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
				string deviceId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string roomId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				int id = sqlite3_column_int(stmt, index++);
				Room *room = gateway->getRoomFromId(roomId);
				Device *device = gateway->getDeviceFromId(deviceId);
				if (!room)
				{
					room = new Room(roomId, id);
					room = gateway->AddNewRoom(room);
				}
				if (!device)
				{
					LOGE("device dose not exist");
				}
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
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (deviceId, roomId, id) VALUES (\"" + device->GetId() + "\",\"" + room->GetUUId() + "\", " + to_string(room->GetId()) + ");";
	return Sqlite_Exec(sql);
}

int Db::DeviceInRoomDel(Room *room, Device *device)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE roomId= \"" + room->GetUUId() + "\" AND deviceId=\"" + device->GetId() + "\";";
	return Sqlite_Exec(sql);
}

int Db::DeviceInRoomDel(Room *room)
{
	string sql = "DELETE FROM " TABLE_NAME " ; ";
	return Sqlite_Exec(sql);
}
