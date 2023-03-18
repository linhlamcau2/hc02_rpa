#include "Db.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"

#define TABLE_NAME "SceneBle"

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
				int addr = sqlite3_column_int(stmt, index++);
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				SceneBle *sceneBle = gateway->getSceneBleFromId(sceneId);
				if (!sceneBle)
				{
					sceneBle = new SceneBle(sceneId, addr, name);
					if (sceneBle)
					{
						gateway->AddNewSceneBle(sceneBle, true, false);
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

int Db::SceneBleAdd(SceneBle *scene)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (scene_ble_id,scene_ble_addr, name) VALUES ('" + scene->GetId() + "', " + to_string(scene->GetAddr()) + ", '" + scene->GetName() + "');";
	return Sqlite_Exec(sql);
}

int Db::SceneBleUpdate(SceneBle *scene)
{
	string sql = "UPDATE " TABLE_NAME " SET scene_ble_addr=" + to_string(scene->GetAddr()) + " AND name='" + scene->GetName() + "';";
	return Sqlite_Exec(sql);
}

int Db::SceneBleDel(SceneBle *scene)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE scene_ble_id='" + scene->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::SceneBleDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
