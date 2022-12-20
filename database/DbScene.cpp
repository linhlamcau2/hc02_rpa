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
				string encode = macaron::Base64::Decode(data, rule);
				if (encode == "")
				{
					LOGV("RuleRead rule: %s", rule.c_str());
					Json::Value ruleValue;
					string errs;
					stringstream s(rule);
					Json::CharReaderBuilder b;
					Json::parseFromStream(b, s, &ruleValue, &errs);
					Rule *rule = gateway->AddRule(ruleValue, true, false);
					rule->Check();
				}
				else
				{
					LOGW("Decode data err: %s", encode.c_str());
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

int Db::RuleAdd(int id, string rule)
{
	string sql = "INSERT INTO " TABLE_NAME " (id, rule) VALUES (" + to_string(id) + ",\"" + macaron::Base64::Encode(rule) + "\");";
	LOGW("RuleAdd: %s", sql.c_str());
	return Sqlite_Exec(sql);
}

int Db::RuleUpdate(int id, string rule)
{
	string sql = "UPDATE " TABLE_NAME " SET rule=\"" + rule + "\" WHERE id=" + to_string(id) + ";";
	return Sqlite_Exec(sql);
}

int Db::RuleDel(int id)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE id=" + to_string(id) + ";";
	return Sqlite_Exec(sql);
}
