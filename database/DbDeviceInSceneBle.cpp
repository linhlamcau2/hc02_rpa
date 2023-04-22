#include "Db.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"
#include "BleDefine.h"

#define TABLE_NAME "[DeviceInSceneBle]"

static int DeviceInSceneBleParse(sqlite3_stmt *stmt, void *ptr)
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
				string sceneBleId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string deviceId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));

				SceneBle *sceneBle = gateway->getSceneBleFromId(sceneBleId);
				Device *device = gateway->getDeviceFromId(deviceId);
				if (sceneBle)
				{
					if (device)
					{
						string devInSceneData;
						string decode = macaron::Base64::Decode(data, devInSceneData);
						if (decode == "")
						{
							Json::Value devInSceneJson;
							if(devInSceneJson.parse(devInSceneData) && devInSceneJson.isArray())
							{
								int modeRgb = 0;
								for (Json::ArrayIndex i = 0; i < devInSceneJson.size(); i++)
								{
									Json::Value property = devInSceneJson[i];
									if (property.isMember("ID") && property["ID"].isInt() && property.isMember("VALUE") && property["VALUE"].isInt())
									{
										if (property["ID"].asInt() == BLE_ATTRIBUTE_SCENE_RGB)
										{
											modeRgb = property["VALUE"].asInt();
											break;
										}
									}
								}
								sceneBle->AddDevice(device, devInSceneJson, modeRgb, true);
							}
							else
							{
								LOGE("data json is not array");
							}
							// TODO: Check cho du lieu V2
						}
						else
						{
							LOGW("Decode data error");
						}
					}
				}
				else
				{
					LOGW("Device or scene does not exist");
				}
			}
			else if (s == SQLITE_DONE)
			{
				return CODE_OK;
			}
			else
			{
				LOGE("DeviceInGroupParse");
				return CODE_ERROR;
			}
		}
	}
	return CODE_OK;
}

int Db::DeviceInSceneBleRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceInSceneBleParse);
}

int Db::DeviceInSceneBleAdd(SceneBle *scene, Device *device, string data)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (scene_ble_id, device_id, data) VALUES ('" + scene->GetId() + "','" + device->GetId() + "','" + macaron::Base64::Encode(data) + "')";
	return Sqlite_Exec(sql);
}

int Db::DeviceInSceneBleDel(SceneBle *scene, Device *device)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE scene_ble_id= '" + scene->GetId() + "' AND device_id='" + device->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::DeviceInSceneBleDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
