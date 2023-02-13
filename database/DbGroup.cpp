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
				string name 		= Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string groupUUId 	= Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				int id = sqlite3_column_int(stmt, index++);
				Group *group = new Group(groupUUId, id, name);
				if(gateway->AddNewGroup(group, true, false))
				{
				}
				else 
				{
					LOGE("AddNewGroup failed");
				}
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
	string sql = "INSERT INTO " TABLE_NAME " (name, groupId, meshId) VALUES ('" + group->GetName() + "','" + group->GetUUId() + "',"+to_string(group->GetId())+")";
	return Sqlite_Exec(sql);
}

int Db::GroupUpdate(Group *group)
{
	string sql = "UPDATE " TABLE_NAME " SET name=\"" + group->GetName() + "\" WHERE id=" + to_string(group->GetId()) + ";";
	return Sqlite_Exec(sql);
}

int Db::GroupDel(Group *group)
{
	return GroupDel(group->GetUUId());
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
