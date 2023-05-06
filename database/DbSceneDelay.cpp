#include "Db.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"

#define TABLE_NAME "[SceneDelay]"

static int SceneDelayParse(sqlite3_stmt *stmt, void *ptr)
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
				int addr = sqlite3_column_int(stmt, index++);
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string sceneData;
				string decode = macaron::Base64::Decode(data, sceneData);
				if (decode == "")
				{
					Json::Value sceneDelayValue;
					if (sceneDelayValue.parse(sceneData) && sceneDelayValue.isObject())
					{
						SceneDelay *sceneDelay = gateway->getSceneDelayFromId(id);
						if (!sceneDelay)
						{
							sceneDelay = new SceneDelay(id, addr, name, sceneDelayValue);
						}
						if (sceneDelay)
						{
							gateway->AddNewSceneDelay(sceneDelay, true, false, true);
						}
					}
					else
					{
						LOGW("SceneDelayRead json format error: %s", sceneData.c_str());
					}
				}
				else
				{
					LOGW("Decode data err: %s", decode.c_str());
				}
			}
			else if (s == SQLITE_DONE)
			{
				return CODE_OK;
			}
			else
			{
				LOGE("SceneDelayParse");
				return CODE_ERROR;
			}
		}
	}
	return CODE_OK;
}

int Db::SceneDelayRead()
{
	return ReadAll(TABLE_NAME, NULL, SceneDelayParse);
}

int Db::SceneDelayAdd(SceneDelay *sceneDelay)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (scene_delay_id, scene_delay_addr, name, data) VALUES ('" + sceneDelay->GetId() + "',"+to_string(sceneDelay->GetAddr())+",'"+sceneDelay->GetName()+"','"+macaron::Base64::Encode(sceneDelay->GetData().toString())+"');";
	return Sqlite_Exec(sql);
}

int Db::SceneDelayUpdateData(SceneDelay *sceneDelay)
{
	string sql = "UPDATE " TABLE_NAME " SET data='" + macaron::Base64::Encode(sceneDelay->GetData().toString()) + "' WHERE scene_delay_id='" + sceneDelay->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::SceneDelayDel(SceneDelay *sceneDelay)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE scene_delay_id='" + sceneDelay->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::SceneDelayDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
