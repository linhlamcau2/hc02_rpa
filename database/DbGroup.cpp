#include "Db.h"
#include "Log.h"
#include "Util.h"

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
				string id = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				int addr = sqlite3_column_int(stmt, index++);
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string roomId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				Group *group = gateway->getGroupFromId(id);
				if (!group)
					group = new Group(id, addr, name);
				if (group)
				{
					if (gateway->AddNewGroup(group, false))
					{
						Room *room = gateway->getRoomFromId(roomId);
						if (room)
						{
							room->AddGroup(group, true, false);
						}
					}
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
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (group_id, name, group_addr) VALUES ('" + group->GetId() + "','" + group->GetName() + "'," + to_string(group->GetAddr()) + ")";
	return Sqlite_Exec(sql);
}

int Db::GroupUpdate(Group *group)
{
	string sql = "UPDATE " TABLE_NAME " SET name='" + group->GetName() + "' WHERE group_id= '" + group->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::GroupUpdateRoom(Group *group, string roomId)
{
	string sql = "UPDATE " TABLE_NAME " SET room_id='" + roomId + "' WHERE group_id= '" + group->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::GroupDel(Group *group)
{
	return GroupDel(group->GetId());
}

int Db::GroupDel(string id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE group_id = \'" + id + "\';";
	return Sqlite_Exec(sql);
}

int Db::GroupDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
