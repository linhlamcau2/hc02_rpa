#include "Db.h"
#include "Log.h"
#include "Util.h"

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
				int addr = sqlite3_column_int(stmt, index++);
				string propertiesData = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				Json::Value payloadJson;
				Json::Reader r;
				r.parse(propertiesData, payloadJson);
				if (payloadJson.isObject())
				{
					SceneBle *scene = gateway->getSceneBleFromId(sceneId);
					Device *device = gateway->getDeviceFromId(deviceId);
					if (scene)
					{
						scene->AddDevice(device, payloadJson, 0, true);
					}
					else
					{
						SceneBle *tempScene = new SceneBle(sceneId, addr, sceneId);
						gateway->AddNewSceneBle(tempScene, true, false);
						tempScene->AddDevice(device, payloadJson, 0, true);
					}
				}
				else
				{
					LOGW("SceneBle json format error data: %s", propertiesData.c_str());
				}
			}
			else if (s == SQLITE_DONE)
			{
				return CODE_OK;
			}
			else
			{
				LOGE("GroupParse");
				return CODE_ERROR;
			}
		}
	}
	return CODE_OK;
}

int Db::SceneBleRead()
{
	return ReadAll(TABLE_NAME, NULL, SceneBleParse);
}

int Db::DeviceInSceneBleAdd(SceneBle *scene, Device *device, Json::Value data)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (sceneId, deviceId, name, meshId, data) VALUES ('" + scene->GetId() + "', '" + device->GetId() + "', '" + scene->GetName() + "', " + to_string(scene->GetAddr()) + ", '" + data.toString() + "');";
	return Sqlite_Exec(sql);
}

int Db::DeviceInSceneBleDel(SceneBle *scene, Device *device, int epId)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE sceneId='" + scene->GetId() + "' AND deviceId='" + device->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::SceneBleDel(SceneBle *scene)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE sceneId='" + scene->GetId() + "';";
	return Sqlite_Exec(sql);
}
