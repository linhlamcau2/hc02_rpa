#include "Db.h"
#include <Log.h>
#include <Util.h>

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
				int id = sqlite3_column_int(stmt, index++);
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string sceneBleUUId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				SceneBle *scene = new SceneBle(sceneBleUUId, id, name);
				// gateway->AddNewSceneBle(scene, true, false);
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

int Db::DevcieInSceneBleAdd(SceneBle *scene, Device *device, int epId, Json::Value data)
{
	// TODO: tuvh
	string sql = "INTER OR REPLACE INTO " TABLE_NAME " (sceneId, deviceId) VALUES ('" + scene->GetUUId() + "', '" + device->GetId() + "');";
	return Sqlite_Exec(sql);
}

int Db::DevcieInSceneBleDel(SceneBle *scene, Device *device, int epId)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE sceneId='" + scene->GetUUId() + "' AND deviceId='" + device->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::SceneBleDel(SceneBle *scene)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE sceneId='" + scene->GetUUId() + "';";
	return Sqlite_Exec(sql);
}