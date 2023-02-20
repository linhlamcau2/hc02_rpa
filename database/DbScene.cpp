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
				index++; // read id
				const string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string rule;
				string decode = macaron::Base64::Decode(data, rule);
				if (decode == "")
				{
					LOGV("RuleRead rule: %s", rule.c_str());
					Json::Value ruleValue;
					Json::Reader r;
					r.parse(rule, ruleValue);
					if (ruleValue.isObject())
					{
						Rule *rule = gateway->AddRule(ruleValue, true, false);
						rule->Check();
					}
					else
					{
						LOGW("RuleRead json format error rule: %s", rule.c_str());
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

int Db::RuleAdd(string id, string rule, string type, bool enable)
{
	string sql = "INSERT INTO " TABLE_NAME " (id, rule, type, enable) VALUES ('" + id + "',\"" + macaron::Base64::Encode(rule) + "\", '"+type+"', "+to_string(enable)+");";
	LOGW("RuleAdd: %s", sql.c_str());
	return Sqlite_Exec(sql);
}

int Db::RuleUpdate(string id, string rule)
{
	string sql = "UPDATE " TABLE_NAME " SET rule=\"" + rule + "\" WHERE id='"+id+"';";
	return Sqlite_Exec(sql);
}

int Db::RuleDel(string id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id='"+id+"';";
	return Sqlite_Exec(sql);
}
