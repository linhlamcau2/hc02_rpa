#include "Db.h"
#include <sys/stat.h>
#include "Log.h"
#ifdef ESP_PLATFORM
#include "esp_littlefs.h"
#endif

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

Db *database = NULL;

Db::Db()
{
	LOGI("Init db");
}

Db::~Db()
{
	if (db)
		sqlite3_close_v2(db);
}

void Db::init(void)
{
#ifdef ESP_PLATFORM
	LOGI("Initializing LITTLEFS");

	esp_vfs_littlefs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = "storage",
		.format_if_mount_failed = true,
		.dont_mount = false,
	};

	// Use settings defined above to initialize and mount LITTLEFS filesystem.
	// Note: esp_vfs_littlefs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_littlefs_register(&conf);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			LOGE("Failed to mount or format filesystem");
		}
		else if (ret == ESP_ERR_NOT_FOUND)
		{
			LOGE("Failed to find LITTLEFS partition");
		}
		else
		{
			LOGE("Failed to initialize LITTLEFS (%s)", esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_littlefs_info(conf.partition_label, &total, &used);
	if (ret != ESP_OK)
	{
		LOGE("Failed to get LITTLEFS partition information (%s)", esp_err_to_name(ret));
	}
	else
	{
		LOGI("Partition size: total: %d, used: %d", total, used);
	}

	sqlite3_initialize();

// // All done, unmount partition and disable LITTLEFS
// esp_vfs_littlefs_unregister(conf.partition_label);
// LOGI("LITTLEFS unmounted");
#endif

	if (pthread_mutex_init(&mutex, NULL) != 0)
	{
		LOGE("Failed to initialize the mutex");
	}

	if (!IsHaveDb())
	{
		sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_JOURNAL, 0);
		createTableIfNotExists();
	}
	else
	{
		sqlite3_open_v2(DB_NAME, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_JOURNAL, 0);
	}
}

bool Db::IsHaveDb()
{
	struct stat st;
	return !stat(DB_NAME, &st);
}

int Db::createTableIfNotExists()
{
#ifdef ESP_PLATFORM
	string sql = "CREATE TABLE IF NOT EXISTS Device (mac VARCHAR, device_id VARCHAR NOT NULL, name VARCHAR, addr INTEGER, type INTEGER, firmware_version VARCHAR, hardware_version VARCHAR, active_time INTEGER, update_time INTEGER, data TEXT,is_favorite INTEGER, PRIMARY KEY (device_id)) WITHOUT ROWID;"
				 "CREATE TABLE IF NOT EXISTS DeviceAttribute (device_id VARCHAR NOT NULL, attribute_id INTEGER, value DOUBLE, PRIMARY KEY (device_id, attribute_id)) WITHOUT ROWID;"
				 "CREATE TABLE IF NOT EXISTS DeviceBleChild (device_id VARCHAR NOT NULL, parent_id VARCHAR NOT NULL, data TEXT, PRIMARY KEY (device_id, parent_id)) WITHOUT ROWID;"
				 "CREATE TABLE IF NOT EXISTS DeviceInGroup (group_id VARCHAR NOT NULL, device_id VARCHAR NOT NULL, element INTEGER, PRIMARY KEY (group_id, device_id, element)) WITHOUT ROWID;"
				 "CREATE TABLE IF NOT EXISTS DeviceInRoom (room_id VARCHAR NOT NULL, device_id VARCHAR NOT NULL, PRIMARY KEY (room_id, device_id)) WITHOUT ROWID;"
				 "CREATE TABLE IF NOT EXISTS DeviceInSceneBle (scene_ble_id VARCHAR NOT NULL, device_id VARCHAR NOT NULL, data TEXT, PRIMARY KEY (scene_ble_id, device_id)) WITHOUT ROWID;"
				 "CREATE TABLE IF NOT EXISTS Gateway (mac VARCHAR NOT NULL ,gateway_id VARCHAR, name VARCHAR, version VARCHAR, ble_netkey VARCHAR, ble_appkey VARCHAR, ble_devicekey VARCHAR, ble_addr INTEGER, ble_iv_index INTEGER, dormitory TEXT, refresh_token TEXT, zigbee_netkey VARCHAR, data TEXT, PRIMARY KEY (mac)) WITHOUT ROWID;"
				 "CREATE TABLE IF NOT EXISTS [Group] (group_id VARCHAR NOT NULL, group_addr INTEGER, name VARCHAR, room_id TEXT, PRIMARY KEY (group_id)) WITHOUT ROWID;"
				 "CREATE TABLE IF NOT EXISTS Room (room_id VARCHAR NOT NULL, room_addr INTEGER, name VARCHAR, data TEXT, PRIMARY KEY (room_id)) WITHOUT ROWID;"
				 "CREATE TABLE IF NOT EXISTS Rule (rule_id VARCHAR NOT NULL, data TEXT NOT NULL, type INTEGER, enable BOOLEAN, rule_addr INTEGER, PRIMARY KEY (rule_id)) WITHOUT ROWID;"
				 "CREATE TABLE IF NOT EXISTS SceneBle (scene_ble_id VARCHAR NOT NULL, scene_ble_addr INTEGER, name VARCHAR, room_id TEXT, is_favorite INTEGER, PRIMARY KEY (scene_ble_id)) WITHOUT ROWID;"
				 "CREATE TABLE IF NOT EXISTS SceneDelay (scene_delay_id VARCHAR NOT NULL, scene_delay_addr INTEGER, name VARCHAR, data TEXT, PRIMARY KEY (scene_delay_id)) WITHOUT ROWID;";
#else
	string sql = "CREATE TABLE IF NOT EXISTS Device (mac VARCHAR, device_id VARCHAR NOT NULL, name VARCHAR, addr INTEGER, type INTEGER, firmware_version VARCHAR, hardware_version VARCHAR, active_time INTEGER, update_time INTEGER, data TEXT,is_favorite INTEGER, PRIMARY KEY (device_id));"
				 "CREATE TABLE IF NOT EXISTS DeviceAttribute (device_id VARCHAR NOT NULL, attribute_id INTEGER, value DOUBLE, PRIMARY KEY (device_id, attribute_id));"
				 "CREATE TABLE IF NOT EXISTS DeviceBleChild (device_id VARCHAR NOT NULL, parent_id VARCHAR NOT NULL, data TEXT PRIMARY KEY (device_id, parent_id));"
				 "CREATE TABLE IF NOT EXISTS DeviceInGroup (group_id VARCHAR NOT NULL, device_id VARCHAR NOT NULL, element INTEGER, PRIMARY KEY (group_id, device_id, element));"
				 "CREATE TABLE IF NOT EXISTS DeviceInRoom (room_id VARCHAR NOT NULL, device_id VARCHAR NOT NULL, PRIMARY KEY (room_id, device_id));"
				 "CREATE TABLE IF NOT EXISTS DeviceInSceneBle (scene_ble_id VARCHAR NOT NULL, device_id VARCHAR NOT NULL, data TEXT, PRIMARY KEY (scene_ble_id, device_id));"
				 "CREATE TABLE IF NOT EXISTS Gateway (mac VARCHAR NOT NULL ,gateway_id VARCHAR, name VARCHAR, version VARCHAR, ble_netkey VARCHAR, ble_appkey VARCHAR, ble_devicekey VARCHAR, ble_addr INTEGER, ble_iv_index INTEGER, dormitory TEXT, refresh_token TEXT, zigbee_netkey VARCHAR, data TEXT, PRIMARY KEY (mac));"
				 "CREATE TABLE IF NOT EXISTS [Group] (group_id VARCHAR NOT NULL, group_addr INTEGER, name VARCHAR, room_id TEXT, PRIMARY KEY (group_id));"
				 "CREATE TABLE IF NOT EXISTS Room (room_id VARCHAR NOT NULL, room_addr INTEGER, name VARCHAR, data TEXT, PRIMARY KEY (room_id));"
				 "CREATE TABLE IF NOT EXISTS Rule (rule_id VARCHAR NOT NULL, data TEXT NOT NULL, type INTEGER, enable BOOLEAN, rule_addr INTEGER, PRIMARY KEY (rule_id));"
				 "CREATE TABLE IF NOT EXISTS SceneBle (scene_ble_id VARCHAR NOT NULL, scene_ble_addr INTEGER, name VARCHAR, room_id TEXT, is_favorite INTEGER, PRIMARY KEY (scene_ble_id));"
				 "CREATE TABLE IF NOT EXISTS SceneDelay (scene_delay_id VARCHAR NOT NULL, scene_delay_addr INTEGER, name VARCHAR, data TEXT, PRIMARY KEY (scene_delay_id));";
#endif
	return Sqlite_Exec(sql);
}

static int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		LOGD("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	LOGD("\n");
	return CODE_OK;
}

int Db::Sqlite_Exec(string &sql)
{
	LOGD("Sqlite_Exec sql: %s", sql.c_str());
	int rc = SQLITE_ERROR;
	char *err_msg = 0;
	if (pthread_mutex_lock(&mutex) == 0)
	{
		rc = sqlite3_exec(db, sql.c_str(), sqlite_callback, NULL, &err_msg);
		if (rc != SQLITE_OK)
		{
			LOGE("Error executing sql statement :%s", err_msg);
			sqlite3_free(err_msg);
		}
		pthread_mutex_unlock(&mutex);
	}
	return rc;
}

int Db::ReadAll(string table, void *listPtr, int (*Parse)(sqlite3_stmt *, void *))
{
	int rc = SQLITE_ERROR;
	sqlite3_stmt *stmt;
	string sql = "SELECT * FROM " + table + ";";

	if (!Parse)
	{
		LOGW("Parse func NULL");
		return CODE_ERROR;
	}

	LOGD("ReadAll table %s", table.c_str());

	if (pthread_mutex_lock(&mutex) == 0)
	{
		rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
		if (rc != SQLITE_OK)
		{
			LOGW("SQL error: %d - %s", rc, sqlite3_errmsg(db));
			pthread_mutex_unlock(&mutex);
			return rc;
		}
		else
		{
			LOGD("sqlite3_prepare_v2 successfully");
		}

		Parse(stmt, listPtr);

		sqlite3_finalize(stmt);
		pthread_mutex_unlock(&mutex);
	}
#ifdef ESP_PLATFORM
	usleep(1000);
#endif
	return rc;
}

int Db::checkAndAddColumn(const std::string &tableName, const std::string &columnNameAdd, string type)
{

	char *errMsg = 0;
	string dataSearch = columnNameAdd + " " + type;

	string sql = "SELECT sql FROM sqlite_master WHERE type='table' AND name='" + tableName + "';";
	sqlite3_stmt *stmt;
	int rs = CODE_ERROR;
	if (db)
	{
		if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK)
		{
			if (sqlite3_step(stmt) == SQLITE_ROW)
			{
				const char *tableDefinition = (const char *)sqlite3_column_text(stmt, 0);
				string tableInfo = string(tableDefinition);
				LOGD("Table [%s] info: %s", tableName.c_str(), tableInfo.c_str());
				if (tableInfo.find(dataSearch) != std::string::npos)
				{
					LOGW("column %s is exist", columnNameAdd.c_str());
					rs = CODE_EXIST;
				}
				else
				{
					LOGD("column %s is not exist", columnNameAdd.c_str());

					const std::string sqlAddColumn = "ALTER TABLE " + tableName + " ADD COLUMN " + columnNameAdd + " " + type + ";";
					int rc = sqlite3_exec(db, sqlAddColumn.c_str(), 0, 0, &errMsg);
					if (rc != SQLITE_OK)
					{
						LOGW("Add column error: %s", errMsg);
						sqlite3_free(errMsg);
					}
					else
					{
						rs = CODE_OK;
					}
				}
			}
			else
			{
				LOGE("Sql error: %s", sql.c_str());
			}
			sqlite3_finalize(stmt);
		}
		else
		{
			LOGW("Sql error: %s", sql.c_str());
		}
	}
	else
	{
		LOGE("Db null");
	}
	return rs;
}
int Db::checkAndDelColumn(const std::string &tableName, const std::string &columnNameDel, string type)
{
	char *errMsg = 0;
	string dataSearch = columnNameDel + " " + type;

	string sql = "SELECT sql FROM sqlite_master WHERE type='table' AND name='" + tableName + "';";
	sqlite3_stmt *stmt;
	int rs = CODE_ERROR;
	if (db)
	{
		if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK)
		{
			if (sqlite3_step(stmt) == SQLITE_ROW)
			{
				const char *tableDefinition = (const char *)sqlite3_column_text(stmt, 0);
				string tableInfo = string(tableDefinition);
				LOGD("Table [%s] info: %s", tableName.c_str(), tableInfo.c_str());
				if (tableInfo.find(dataSearch) != std::string::npos)
				{
					LOGD("column %s is exist", columnNameDel.c_str());

					const std::string sqlAddColumn = "ALTER TABLE " + tableName + " DROP COLUMN " + columnNameDel + ";";
					int rc = sqlite3_exec(db, sqlAddColumn.c_str(), 0, 0, &errMsg);
					if (rc != SQLITE_OK)
					{
						LOGW("Del column error: %s", errMsg);
						sqlite3_free(errMsg);
					}
					else
					{
						rs = CODE_OK;
					}
				}
				else
				{
					LOGW("column %s is not exist", columnNameDel.c_str());
					rs = CODE_NOT_EXIST;
				}
			}
			else
			{
				LOGE("Sql error: %s", sql.c_str());
			}
			sqlite3_finalize(stmt);
		}
		else
		{
			LOGW("Sql error: %s", sql.c_str());
		}
	}
	else
	{
		LOGE("Db null");
	}
	return rs;
}