#include "Db.h"
#include <Log.h>

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

Db *database = NULL;

Db::Db()
{
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
	int rc;
	sqlite3 *db;
	char *err_msg = 0;
	mtx.lock();
	rc = sqlite3_open(DB_NAME, &db);
	if (rc)
	{
		LOGE("Can't open database: %s", sqlite3_errmsg(db));
		mtx.unlock();
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
	mtx.unlock();
	return rc;
}

int Db::ReadAll(string table, void *listPtr, int (*Parse)(sqlite3_stmt *, void *))
{
	int rc;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	string sql = "SELECT * FROM " + table + ";";

	if (!Parse)
	{
		LOGW("Parse func NULL");
		return 1;
	}
	
	LOGD("ReadAll table %s", table.c_str());

	mtx.lock();
	rc = sqlite3_open(DB_NAME, &db);
	if (rc)
	{
		LOGE("Can't open database: %s", sqlite3_errmsg(db));
		mtx.unlock();
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
		mtx.unlock();
		return rc;
	}
	else
	{
		LOGD("sqlite3_prepare_v2 successfully");
	}

	Parse(stmt, listPtr);

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	mtx.unlock();
	return rc;
}
