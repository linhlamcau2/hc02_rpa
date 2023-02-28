#include "Db.h"
#include <Log.h>
#include <Util.h>
#include <Base64.h>
#include "room/Room.h"

#define TABLE_NAME "[DataRoom]"

static int DataRoomParse(sqlite3_stmt *stmt, void *ptr)
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
				string id = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
                Room * room = gateway->getRoomFromId(id);
                string roomdata ;
                string decode = macaron::Base64::Decode(data, roomdata);
				LOGE("room data raw: %s", roomdata.c_str());
                if (decode == "")
                {
                    Json::Value roomValue;
					Json::Reader r;
					r.parse(roomdata, roomValue);
                    if (roomValue.isObject())
					{
						LOGI("Room data config: %s", roomValue.toString().c_str());
                        if (room)
                            room->DataConfigAdd(roomValue.toString());
                        else
                        {
                            LOGW("Room does not exsit");
                        }
                    }
					else
					{
						LOGW("Room data config error %s", roomValue.toString().c_str());
					}
                }
				else
				{
					LOGW("decode error");
				}
			}
			else if (s == SQLITE_DONE)
			{
				return 0;
			}
			else
			{
				LOGE("Data Room Parse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::DataRoomRead()
{
	return ReadAll(TABLE_NAME, NULL, DataRoomParse);
}

int Db::DataRoomAdd(string id, string data)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (id, data) VALUES (\"" + id + "\",\"" + macaron::Base64::Encode(data) + "\")";
	return Sqlite_Exec(sql);
}

int Db::DataRoomUpdate(string id, string data)
{
	string sql = "UPDATE " TABLE_NAME " SET id=\"" + id + "\", data=\"" + data + "\";";
	return Sqlite_Exec(sql);
}

int Db::DataRoomDel(string id, string data)
{
    string sql = "DELETE FROM " TABLE_NAME " WHERE id = \"" + id+"\" AND data = \"" + data +"\";";
	return Sqlite_Exec(sql);
}

int Db::DataRoomDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
