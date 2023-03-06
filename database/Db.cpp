#include "Db.h"
#include <Log.h>
#ifdef ESP_PLATFORM
#include "esp_spiffs.h"
#endif

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

Db *database = NULL;

Db::Db()
{
}

void Db::init(void)
{
	if (pthread_mutex_init(&mutex, NULL) != 0)
	{
		LOGE("Failed to initialize the mutex");
	}
#ifdef ESP_PLATFORM
	LOGI("Initializing SPIFFS");

	esp_vfs_spiffs_conf_t conf = {
			.base_path = "/spiffs",
			.partition_label = NULL,
			.max_files = 5,
			.format_if_mount_failed = true};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			LOGE("Failed to mount or format filesystem");
		}
		else if (ret == ESP_ERR_NOT_FOUND)
		{
			LOGE("Failed to find SPIFFS partition");
		}
		else
		{
			LOGE("Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(conf.partition_label, &total, &used);
	if (ret != ESP_OK)
	{
		LOGE("Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
	}
	else
	{
		LOGI("Partition size: total: %d, used: %d", total, used);
	}

	sqlite3_initialize();

// // All done, unmount partition and disable SPIFFS
// esp_vfs_spiffs_unregister(conf.partition_label);
// LOGI("SPIFFS unmounted");
#endif
}

static int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		LOGD("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	LOGD("\n");
	return 0;
}

int Db::Sqlite_Exec(string &sql)
{
	LOGD("Sqlite_Exec sql: %s", sql.c_str());
	int rc = SQLITE_ERROR;
	sqlite3 *db;
	char *err_msg = 0;
	if (pthread_mutex_lock(&mutex) == 0)
	{
		rc = sqlite3_open(DB_NAME, &db);
		if (rc)
		{
			LOGE("Can't open database: %s", sqlite3_errmsg(db));
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		else
		{
			// LOGD("Opened database successfully");
		}
		rc = sqlite3_exec(db, sql.c_str(), sqlite_callback, NULL, &err_msg);
		if (rc != SQLITE_OK)
		{
			LOGE("Error executing sql statement :%s", err_msg);
			sqlite3_free(err_msg);
		}
		sqlite3_close(db);
		pthread_mutex_unlock(&mutex);
	}
	return rc;
}

int Db::ReadAll(string table, void *listPtr, int (*Parse)(sqlite3_stmt *, void *))
{
	int rc = SQLITE_ERROR;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	string sql = "SELECT * FROM " + table + ";";

	if (!Parse)
	{
		LOGW("Parse func NULL");
		return 1;
	}

	LOGD("ReadAll table %s", table.c_str());

	if (pthread_mutex_lock(&mutex) == 0)
	{
		rc = sqlite3_open(DB_NAME, &db);
		if (rc)
		{
			LOGE("Can't open database: %s", sqlite3_errmsg(db));
			pthread_mutex_unlock(&mutex);
			return rc;
		}
		else
		{
			LOGD("Opened database successfully");
		}

		rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
		if (rc != SQLITE_OK)
		{
			LOGW("SQL error: %d - %s", rc, sql.c_str());
			sqlite3_close(db);
			pthread_mutex_unlock(&mutex);
			return rc;
		}
		else
		{
			LOGD("sqlite3_prepare_v2 successfully");
		}

		Parse(stmt, listPtr);

		sqlite3_finalize(stmt);
		sqlite3_close(db);
		pthread_mutex_unlock(&mutex);
	}
	return rc;
}
