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
				string sceneId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string deviceId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				int meshId = sqlite3_column_int(stmt, index++);
				string propertiesData = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				Json::Value payloadJson;
				Json::Reader r;
				r.parse(propertiesData, payloadJson);
				if (payloadJson.isArray())
				{
					SceneBle *scene = gateway->getSceneBleFromId(sceneId);
					Device *device = gateway->getDevice(deviceId);
					cout << "sceneId: " + sceneId + "---data: " + propertiesData << endl;
					if (scene)
					{
						scene->AddDevice(device, payloadJson, 0, true);
						gateway->AddNewSceneBle(scene, true, false);
					}
					else
					{
						SceneBle *tempScene = new SceneBle(sceneId, meshId, sceneId);
						gateway->AddNewSceneBle(tempScene, true, false);
					}
				}
				else
				{
					LOGW("SceneBle json format error data: %s", propertiesData.c_str());
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

int Db::DeviceInSceneBleAdd(SceneBle *scene, Device *device, Json::Value data)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (sceneId, deviceId, name, meshId, data) VALUES ('" + scene->GetUUId() + "', '" + device->GetId() + "', '" + scene->GetName() + "', " + to_string(scene->GetId()) + ", '" + data.toString() + "');";
	return Sqlite_Exec(sql);
}

int Db::DeviceInSceneBleDel(SceneBle *scene, Device *device, int epId)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE sceneId='" + scene->GetUUId() + "' AND deviceId='" + device->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::SceneBleDel(SceneBle *scene)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE sceneId='" + scene->GetUUId() + "';";
	return Sqlite_Exec(sql);
}