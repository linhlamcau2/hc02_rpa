#include "Db.h"
#include "Log.h"
#include "Util.h"

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
				int id = sqlite3_column_int(stmt, index++);
                Room * room = new Room(roomId, id);
                if (!gateway->AddNewRoom(room))
                {
                    LOGE("Add new room failed");
                }
			}
			else if (s == SQLITE_DONE)
			{
				return 0;
			}
			else
			{
				LOGE("RoomParse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::RoomRead()
{
	return ReadAll(TABLE_NAME, NULL, RoomParse);
}

int Db::RoomAdd(Room *room)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (roomId, addr) VALUES ('" + room->GetUUId() + "'," + to_string(room->GetId()) + ")";
	return Sqlite_Exec(sql);
}

int Db::RoomUpdate(Room *room, int id)
{
	string sql = "UPDATE " TABLE_NAME " SET addr=" + to_string(id) + " WHERE roomId = \"" + room->GetUUId() + "\";";
	return Sqlite_Exec(sql);
}

int Db::RoomDel(Room *room)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE roomId = \'" + room->GetUUId() + "\';";
	return Sqlite_Exec(sql);
}

int Db::RoomDelAll()
{
    string sql = "DELETE FROM " TABLE_NAME ";";
    return Sqlite_Exec(sql);
}
