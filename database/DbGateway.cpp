#include "Db.h"
#include "Log.h"
#include "Util.h"

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
				string mac = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string id = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string version = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string ble_netkey = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string ble_appkey = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string ble_devicekey = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				uint16_t ble_addr = sqlite3_column_int(stmt, index++);
				uint32_t ble_iv_index = sqlite3_column_int(stmt, index++);
				string dormitory = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string refresh = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string zigbee_netkey = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));

				gateway->setMac(mac);
				gateway->setId(id);
				gateway->setName(name);
				gateway->setBleNetkey(ble_netkey);
				gateway->setBleAppkey(ble_appkey);
				gateway->setBleDevicekey(ble_devicekey);
				gateway->setBleAddr(ble_addr);
				gateway->setBleIvIndex(ble_iv_index);
				gateway->setDormitory(dormitory);
				gateway->setRefreshToken(refresh);

				string versionCode = STR(VERSION);
				if (version == "" && versionCode == "2.0.0")
				{
					version = versionCode;
				}
				gateway->setVersion(version);

				LOGI("Gateway mac: %s, id: %s, name: %s, version: %s , ble_netkey: %s,ble_appkey: %s, ble_devicekey: %s, ble_addr: %d, ble_iv_index: %d, dormitory: %s, refresh_token: %s",
					 gateway->getMac().c_str(), gateway->getId().c_str(), gateway->getName().c_str(), gateway->getVersion().c_str(), gateway->getBleNetKey().c_str(), gateway->getBleAppKey().c_str(),
					 gateway->getBleDeviceKey().c_str(), gateway->getBleAddr(), gateway->getBleIvIndex(), gateway->getDormitory().c_str(), gateway->getRefreshToken().c_str());

				return CODE_OK;
			}
			else if (s == SQLITE_DONE)
			{
				return CODE_OK;
			}
			else
			{
				LOGE("GatewayParse");
				return CODE_ERROR;
			}
		}
	}
	return CODE_OK;
}

int Db::GatewayRead()
{
	return ReadAll(TABLE_NAME, NULL, GatewayParse);
}

int Db::GatewayAdd(Gateway *gateway)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (mac, gateway_id, name, version, ble_netkey, ble_appkey, ble_devicekey, ble_addr, ble_iv_index, dormitory,refresh_token) VALUES ('" + gateway->getMac() + "','" + gateway->getId() + "','" + gateway->getName() + "','" + gateway->getVersion() + "','" + gateway->getBleNetKey() + "','" + gateway->getBleAppKey() + "','" + gateway->getBleDeviceKey() + "'," + to_string(gateway->getBleAddr()) + "," + to_string(gateway->getBleIvIndex()) + ",'" + gateway->getDormitory() + "','" + gateway->getRefreshToken() + "')";
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdateId(Gateway *gateway, string id)
{
	string sql = "UPDATE " TABLE_NAME " SET gateway_id = '" + id + "' WHERE mac = '" + gateway->getMac() + "'";
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdateNetKey(Gateway *gateway, string netkey)
{
	string sql = "UPDATE " TABLE_NAME " SET ble_netkey='" + netkey + "' WHERE mac='" + gateway->getMac() + "';";
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdateAppKey(Gateway *gateway, string appkey)
{
	string sql = "UPDATE " TABLE_NAME " SET ble_appkey='" + appkey + "' WHERE mac='" + gateway->getMac() + "';";
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdateDeviceKey(Gateway *gateway, string devicekey)
{
	string sql = "UPDATE " TABLE_NAME " SET ble_devicekey='" + devicekey + "' WHERE mac='" + gateway->getMac() + "';";
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdateUnicast(Gateway *gateway, uint16_t unicast)
{
	string sql = "UPDATE " TABLE_NAME " SET ble_addr =" + to_string(unicast) + " WHERE mac='" + gateway->getMac() + "';";
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdateIvIndex(Gateway *gateway, uint32_t iv_index)
{
	string sql = "UPDATE " TABLE_NAME " SET ble_iv_index =" + to_string(iv_index) + " WHERE mac='" + gateway->getMac() + "';";
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdateDormitory(Gateway *gateway, string dormitory)
{
	string sql = "UPDATE " TABLE_NAME " SET dormitory='" + dormitory + "' WHERE mac='" + gateway->getMac() + "';";
	return Sqlite_Exec(sql);
}

int Db::GatewayUpdateRefreshToken(Gateway *gateway, string refreshToken)
{
	string sql = "UPDATE " TABLE_NAME " SET refresh_token='" + refreshToken + "' WHERE mac='" + gateway->getMac() + "';";
	return Sqlite_Exec(sql);
}

int Db::GatewayDel(Gateway *gateway)
{
	return GatewayDel(gateway->getMac());
}

int Db::GatewayDel(string mac)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE mac='" + mac + "';";
	return Sqlite_Exec(sql);
}

int Db::GatewayDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
