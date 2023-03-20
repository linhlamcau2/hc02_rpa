#include "Db.h"
#include "Log.h"
#include "Util.h"

#define TABLE_NAME "[Grouping]"

static int GroupParse(sqlite3_stmt *stmt, void *ptr)
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
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string id = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				int addr = sqlite3_column_int(stmt, index++);
				Group *group = new Group(id, addr, name);
				if (gateway->AddNewGroup(group, true, false))
				{
				}
				else
				{
					LOGE("AddNewGroup failed");
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

int Db::GroupRead()
{
	return ReadAll(TABLE_NAME, NULL, GroupParse);
}

int Db::GroupAdd(Group *group)
{
	string sql = "INSERT INTO " TABLE_NAME " (name, groupId, meshId) VALUES ('" + group->GetName() + "','" + group->GetId() + "'," + to_string(group->GetAddr()) + ");";
	return Sqlite_Exec(sql);
}

int Db::GroupUpdate(Group *group)
{
	string sql = "UPDATE " TABLE_NAME " SET name=\"" + group->GetName() + "\" WHERE id=" + to_string(group->GetAddr()) + ";";
	return Sqlite_Exec(sql);
}

int Db::GroupDel(Group *group)
{
	return GroupDel(group->GetId());
}

int Db::GroupDel(string id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE groupId = \'" + id + "\';";
	return Sqlite_Exec(sql);
}

int Db::GroupDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
