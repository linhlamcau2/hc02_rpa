#include "Db.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"

#define TABLE_NAME "[SceneBle]"

static int SceneBleParse(sqlite3_stmt *stmt, void *ptr)
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
				string sceneId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				uint16_t addr = sqlite3_column_int(stmt, index++);
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string roomId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				bool isFavorite = sqlite3_column_int(stmt, index++);
				SceneBle *sceneBle = new SceneBle(sceneId, addr, name);
				if (sceneBle)
				{
					sceneBle->SetIsFavorite(isFavorite);
					if (gateway->AddNewSceneBle(sceneBle, false))
					{
						Room *room = gateway->getRoomFromId(roomId);
						if (room)
						{
							room->AddSceneBle(sceneBle, true, false);
						}
					}
				}
			}
			else if (s == SQLITE_DONE)
			{
				return 0;
			}
			else
			{
				LOGE("GroupParse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::SceneBleRead()
{
	return ReadAll(TABLE_NAME, NULL, SceneBleParse);
}

int Db::SceneBleAdd(SceneBle *sceneBle)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (scene_ble_id,scene_ble_addr, name) VALUES ('" + sceneBle->GetId() + "', " + to_string(sceneBle->GetAddr()) + ", '" + sceneBle->GetName() + "');";
	return Sqlite_Exec(sql);
}

int Db::SceneBleUpdate(SceneBle *sceneBle)
{
	string sql = "UPDATE " TABLE_NAME " SET name='" + sceneBle->GetName() + "' WHERE scene_ble_id='" + sceneBle->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::SceneBleUpdateRoom(SceneBle *sceneBle, string roomId)
{
	string sql = "UPDATE " TABLE_NAME " SET room_id='" + roomId + "' WHERE scene_ble_id ='" + sceneBle->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::SceneBleDel(SceneBle *sceneBle)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE scene_ble_id='" + sceneBle->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::SceneBleDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}

int Db::SceneBleUpdateFavorite(SceneBle *scene)
{
	string sql = "UPDATE " TABLE_NAME " SET is_favorite= " + to_string(scene->GetIsFavorite()) + " WHERE scene_ble_id='" + scene->GetId() + "';";
	return Sqlite_Exec(sql);
}
