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
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string version = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string ble_netkey = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string ble_appkey = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string ble_devicekey = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				uint16_t ble_unicast = sqlite3_column_int(stmt, index++);
				string dormitory = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string zigbee_netkey = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				LOGI("Gateway: id: %s, version: %s, name: %s, appkey: %s, netkey: %s, devicekey: %s, unicast: %d, dormitory: %s", id.c_str(), version.c_str(), name.c_str(), ble_appkey.c_str(), ble_netkey.c_str(), ble_devicekey.c_str(), ble_unicast, dormitory.c_str());

				gateway->setId(id);
				gateway->setBleAppkey(ble_appkey);
				gateway->setBleDevicekey(ble_devicekey);
				gateway->setBleNetkey(ble_netkey);
				gateway->setBleUnicast(ble_unicast);
				gateway->setDormitory(dormitory);
				return 0;
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
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (id, name, version, ble_netkey, ble_appkey, ble_devicekey, dormitory) VALUES (\"" + gateway->getId() + "\",\"" + gateway->getName() + "\",\"" + gateway->getVersion() + "\",\"" + gateway->getBleNetkey() + "\",\"" + gateway->getBleAppKey() + "\",\"" + gateway->getBleDeviceKey() + "\",\"" + gateway->getDormitory() + "\")";
	LOGD("sql: %s", sql.c_str());
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdate(Gateway *gateway)
{
	string sql = "UPDATE " TABLE_NAME " SET name=\"" + gateway->getName() + "\", version=\"" + gateway->getVersion() + "\", ble_netkey=\"" + gateway->getBleNetkey() + "\", ble_appkey=\"" + gateway->getBleAppKey() + "\", ble_devicekey=\"" + gateway->getBleDeviceKey() + "\", dormitory=\"" + gateway->getDormitory() + "\"  WHERE mac=\"" + gateway->getId() + "\";";
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdateId(Gateway *gateway, string id)
{
	string sql = "INSERT INTO " TABLE_NAME " (id) VALUES (\""+id+"\");";
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdateNetKey(Gateway *gateway, string netkey)
{
	string sql = "UPDATE " TABLE_NAME " SET ble_netkey=\"" + netkey + "\" WHERE id=\"" + gateway->getId() + "\";";
	return Sqlite_Exec(sql);
}
int Db::GatewayUpdateAppKey(Gateway *gateway, string appkey)
{
	string sql = "UPDATE " TABLE_NAME " SET ble_appkey=\"" + appkey + "\" WHERE id=\"" + gateway->getId() + "\";";
	return Sqlite_Exec(sql);
}
int Db::GatewayUpdateDeviceKey(Gateway *gateway, string devicekey)
{
	string sql = "UPDATE " TABLE_NAME " SET ble_devicekey=\"" + devicekey + "\" WHERE id=\"" + gateway->getId() + "\";";
	return Sqlite_Exec(sql);
}
int Db::GatewayUpdateUnicast(Gateway *gateway, uint16_t unicast)
{
	string sql = "UPDATE " TABLE_NAME " SET ble_unicast =" + to_string(unicast) + " WHERE id=\"" + gateway->getId() + "\";";
	return Sqlite_Exec(sql);
}
int Db::GatewayUpdateDormitory(Gateway *gateway, string dormitory)
{
	string sql = "UPDATE " TABLE_NAME " SET dormitory=\"" + dormitory + "\" WHERE id=\"" + gateway->getId() + "\";";
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
