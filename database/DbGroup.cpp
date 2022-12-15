#include "Db.h"
#include <Log.h>
#include <Util.h>

#define TABLE_NAME "[Group]"

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
				int id = sqlite3_column_int(stmt, index++);
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string groupUUId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				Group *group = new Group(groupUUId, id, name);
				gateway->AddNewGroup(group, true, false);
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

int Db::GroupRead()
{
	return ReadAll(TABLE_NAME, NULL, GroupParse);
}

int Db::GroupAdd(Group *group)
{
	string sql = "INSERT INTO " TABLE_NAME " (id, name) VALUES (" + to_string(group->GetId()) + ",\"" + group->GetName() + "\")";
	return Sqlite_Exec(sql);
}

int Db::GroupUpdate(Group *group)
{
	string sql = "UPDATE " TABLE_NAME " SET name=\"" + group->GetName() + "\" WHERE id=" + to_string(group->GetId()) + ";";
	return Sqlite_Exec(sql);
}

int Db::GroupDel(Group *group)
{
	return GroupDel(group->GetId());
}

int Db::GroupDel(int id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id=" + to_string(id) + ";";
	return Sqlite_Exec(sql);
}

int Db::GroupDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
