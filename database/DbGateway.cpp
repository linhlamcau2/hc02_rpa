#include "Db.h"
#include <Log.h>
#include <Util.h>

#define TABLE_NAME "[Gateway]"

static int GatewayParse(sqlite3_stmt *stmt, void *ptr)
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
				string version = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string ble_netkey = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string ble_appkey = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string ble_devicekey = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string dormitory = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				gateway->setBleAppkey(ble_appkey);
				gateway->setBleDevicekey(ble_devicekey);
				gateway->setBleNetkey(ble_netkey);
				gateway->setDormitory(dormitory);
			}
			else if (s == SQLITE_DONE)
			{
				return 0;
			}
			else
			{
				LOGE("GatewayParse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::GatewayRead()
{
	return ReadAll(TABLE_NAME, NULL, GatewayParse);
}

int Db::GatewayAdd(Gateway *gateway)
{
	string sql = "INSERT INTO " TABLE_NAME " (id, name, version, ble_netkey, ble_appkey, ble_devicekey, dormitory) VALUES (\"" + gateway->getId() + "\",\"" + gateway->getName() + "\",\"" + gateway->getVersion() + "\",\"" + gateway->getBleNetkey() + "\",\"" + gateway->getBleAppKey() + "\",\"" + gateway->getBleDeviceKey() + "\",\"" + gateway->getDormitory() + "\")";
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdate(Gateway *gateway)
{
	string sql = "UPDATE " TABLE_NAME " SET name=\"" + gateway->getName() + "\", version=\"" + gateway->getVersion() + "\", ble_netkey=\"" + gateway->getBleNetkey() + "\", ble_appkey=\""+gateway->getBleAppKey()+"\", ble_devicekey=\""+gateway->getBleDeviceKey()+"\", dormitory=\""+gateway->getDormitory()+"\"  WHERE mac=\"" + gateway->getId() + "\";";
	return Sqlite_Exec(sql);
}

int Db::GatewayDel(Gateway *gateway)
{
	return DeviceDel(gateway->getId());
}

int Db::GatewayDel(string id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id=\"" + id + "\";";
	return Sqlite_Exec(sql);
}

int Db::GatewayDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}