#include "Db.h"
#include <Log.h>
#include <Util.h>
#include <Base64.h>

#define TABLE_NAME "[Scene]"

static int SceneParse(sqlite3_stmt *stmt, void *ptr)
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
				index++; // read id
				const string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string scene;
				string encode = macaron::Base64::Decode(data, scene);
				if (encode == "")
				{
					LOGV("SceneRead scene: %s", scene.c_str());
					Json::Value sceneValue;
					string errs;
					stringstream s(scene);
					Json::CharReaderBuilder b;
					Json::parseFromStream(b, s, &sceneValue, &errs);
					Scene *scene = gateway->AddScene(sceneValue, true, false);
					scene->Check();
				}
				else
				{
					LOGW("Decode data err: %s", encode.c_str());
				}
			}
			else if (s == SQLITE_DONE)
			{
				return 0;
			}
			else
			{
				LOGE("SceneParse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::SceneRead()
{
	return ReadAll(TABLE_NAME, NULL, SceneParse);
}

int Db::SceneAdd(int id, string scene)
{
	string sql = "INSERT INTO " TABLE_NAME " (id, scene) VALUES (" + to_string(id) + ",\"" + macaron::Base64::Encode(scene) + "\");";
	LOGW("SceneAdd: %s", sql.c_str());
	return Sqlite_Exec(sql);
}

int Db::SceneUpdate(int id, string scene)
{
	string sql = "UPDATE " TABLE_NAME " SET scene=\"" + scene + "\" WHERE id=" + to_string(id) + ";";
	return Sqlite_Exec(sql);
}

int Db::SceneDel(int id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id=" + to_string(id) + ";";
	return Sqlite_Exec(sql);
}


