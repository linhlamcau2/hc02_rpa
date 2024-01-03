#include "Db.h"
#include "Log.h"
#include "Util.h"

#define TABLE_NAME "[Noti]"

static int NotiParse(sqlite3_stmt *stmt, void *ptr)
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
                string type = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				bool isRead = sqlite3_column_int(stmt, index++) ? true : false;
				string content = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				long updateAt = sqlite3_column_int(stmt, index++);
                long createAt = sqlite3_column_int(stmt, index++);
				
				Noti *noti = new Noti(id, type, content, to_string(updateAt), to_string(createAt));
				if (noti)
				{
					gateway->CreateNoti(noti, false);
				}
			}
			else if (s == SQLITE_DONE)
			{
				return CODE_OK;
			}
			else
			{
				LOGE("NotiParse");
				return CODE_ERROR;
			}
		}
	}
	return CODE_OK;
}

int Db::NotiRead()
{
    return ReadAll(TABLE_NAME, NULL, NotiParse);
}

int Db::NotiAdd(Noti *noti)
{
    string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (id, type, is_read, content, update_time, create_time) VALUES ('" + noti->GetId() + "','" + noti->GetType() + "'," + to_string(noti->GetIsRead()) + ", '" + noti->GetContent() + "', " + to_string(time(NULL)) + ", " + to_string(time(NULL)) + ")";
	return Sqlite_Exec(sql);
}

int Db::NotiUpdate(Noti *noti)
{
    string sql = "UPDATE " TABLE_NAME " SET is_read=" + to_string(noti->GetIsRead()) + " WHERE id= '" + noti->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::NotiDel(Noti *noti)
{
    string sql = "DELETE FROM " TABLE_NAME " WHERE id = '" + noti->GetId() + "';";
	return Sqlite_Exec(sql);
}
