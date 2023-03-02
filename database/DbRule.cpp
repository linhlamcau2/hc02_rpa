#include "Db.h"
#include <Log.h>
#include <Util.h>
#include <Base64.h>

#define TABLE_NAME "[Rule]"

static int RuleParse(sqlite3_stmt *stmt, void *ptr)
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
				string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				int status = sqlite3_column_int(stmt, index++);
				int type = sqlite3_column_int(stmt, index++);
				string ruledata ;
				LOGW("Rule raw: %s", data.c_str());
				string decode = macaron::Base64::Decode(data, ruledata);
				if ((decode == "") && status)
				{
					LOGV("RuleRead rule: %s", ruledata.c_str());
					Json::Value ruleValue;
					Json::Reader r;
					r.parse(ruledata, ruleValue);
					if (ruleValue.isObject())
					{
						Rule *rule = gateway->AddRule(ruleValue, true, false);
						rule->Check();
					}
					else
					{
						LOGW("RuleRead json format error rule: %s", ruledata.c_str());
					}
				}
				else
				{
					LOGW("Decode data err: %s", decode.c_str());
				}
			}
			else if (s == SQLITE_DONE)
			{
				return 0;
			}
			else
			{
				LOGE("RuleParse");
				return 1;
			}
		}
	}
	return 0;
}

int Db::RuleRead()
{
	return ReadAll(TABLE_NAME, NULL, RuleParse);
}

int Db::RuleAdd(string id, string rule, int isEnable, int type)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (id, rule, isEnable, type) VALUES (\"" + id + "\",\"" + macaron::Base64::Encode(rule) + "\"," + to_string(isEnable) + "," + to_string(type) + ");";
	return Sqlite_Exec(sql);
}

int Db::RuleUpdate(string id, string rule)
{
	string sql = "UPDATE " TABLE_NAME " SET rule=\"" + rule + "\" WHERE id=\"" + id + "\";";
	return Sqlite_Exec(sql);
}
int Db::RuleUpdateStatus(string id, int isEnable)
{
	string sql = "UPDATE " TABLE_NAME "SET isEnable="+to_string(isEnable)+" WHERE id=\"" + id + "\";";
	return Sqlite_Exec(sql);
}

int Db::RuleDel(string id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id=\"" + id + "\";";
	return Sqlite_Exec(sql);
}
