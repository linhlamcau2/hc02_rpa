#include "Db.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"

#define TABLE_NAME "[Room]"

static int RoomParse(sqlite3_stmt *stmt, void *ptr)
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
				uint16_t addr = sqlite3_column_int(stmt, index++);
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				Room *room = new Room(roomId, addr, name);
				if (room)
				{
					string roomData;
					string decode = macaron::Base64::Decode(data, roomData);
					if (decode == "")
					{
						Json::Value roomValue;
						if (roomValue.parse(roomData) && roomValue.isObject())
						{
							room->SetDataConfig(roomValue.toString());
						}
					}
					else
					{
						LOGW("Decode data error: %s", data.c_str());
					}
					if (!gateway->AddNewRoom(room, false))
					{
						LOGE("Add new room failed");
					}
				}
			}
			else if (s == SQLITE_DONE)
			{
				return CODE_OK;
			}
			else
			{
				LOGE("RoomParse");
				return CODE_ERROR;
			}
		}
	}
	return CODE_OK;
}

int Db::RoomRead()
{
	return ReadAll(TABLE_NAME, NULL, RoomParse);
}

int Db::RoomAdd(Room *room)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (room_id, room_addr, name, data) VALUES ('" + room->GetId() + "'," + to_string(room->GetAddr()) + ",'" + room->GetName() + "','" + macaron::Base64::Encode(room->GetDataConfig()) + "');";
	return Sqlite_Exec(sql);
}

int Db::RoomUpdate(Room *room, int id)
{
	string sql = "UPDATE " TABLE_NAME " SET addr=" + to_string(id) + " WHERE room_id = '" + room->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::RoomDel(Room *room)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE room_id = \'" + room->GetId() + "\';";
	return Sqlite_Exec(sql);
}

int Db::RoomDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
