#include "Db.h"
#include "Log.h"
#include "Util.h"
#include "DeviceBleSeftPowerRemote.h"

#define TABLE_NAME "[DeviceBleChild]"

static int DeviceBleChildParse(sqlite3_stmt *stmt, void *ptr)
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
				string deviceId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string parentId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				LOGD("deviceId: %s, parentId: %s, data: %s", deviceId.c_str(), parentId.c_str(), data.c_str());
				Device *child = gateway->getDeviceFromId(deviceId);
				Device *parent = gateway->getDeviceFromId(parentId);
				if (parent)
				{
					if (child)
					{
						DeviceBleSeftPowerRemote *deviceBleSeftPowerRemote = dynamic_cast<DeviceBleSeftPowerRemote *>(child);
						if (deviceBleSeftPowerRemote)
							deviceBleSeftPowerRemote->SetParentDev(parent);
					}
					else
						LOGW("child device not found");
				}
				else
					LOGW("parent device not found");
			}
			else if (s == SQLITE_DONE)
			{
				return CODE_OK;
			}
			else
			{
				LOGE("Device child parese");
				return CODE_ERROR;
			}
		}
	}
	return CODE_OK;
}

int Db::DeviceBleChildRead()
{
	return ReadAll(TABLE_NAME, NULL, DeviceBleChildParse);
}

int Db::DeviceBleChildAdd(Device * child, Device * parent, string data)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (device_id, parent_id, data) VALUES ('" + child->GetId() + "','" + parent->GetId() + "','"+data+"');";
	return Sqlite_Exec(sql);
}

int Db::DeviceBleChildUpdateData(Device * child, Device * parent, string data)
{
	string sql = "UPDATE " TABLE_NAME " SET data='" + data + "' where device_id = '"+child->GetId()+"' AND parent_id = '"+parent->GetId()+"';";
	return Sqlite_Exec(sql);
}

int Db::DeviceBleChildDel(Device * child, Device * parent)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE device_id ='" + child->GetId() + "' AND parent_id='"+parent->GetId()+"';";
	return Sqlite_Exec(sql);
}

int Db::DeviceBleChildDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
