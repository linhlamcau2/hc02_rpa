#include "Db.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"

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
				int type = sqlite3_column_int(stmt, index++);
				bool enable = sqlite3_column_blob(stmt, index++);
				int addr = sqlite3_column_int(stmt, index++);
				string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
				string ruledata;
				string decode = macaron::Base64::Decode(data, ruledata);
				if (decode == "")
				{
					Json::Value ruleValue;
					if (ruleValue.parse(ruledata) && ruleValue.isObject())
					{
						Rule *rule = gateway->AddRule(ruleValue, true, false);
						if (rule)
						{
							rule->SetStatus(enable);
							rule->Check();
						}
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
				return CODE_OK;
			}
			else
			{
				LOGE("RuleParse");
				return CODE_ERROR;
			}
		}
	}
	return CODE_OK;
}

int Db::RuleRead()
{
	return ReadAll(TABLE_NAME, NULL, RuleParse);
}

int Db::RuleAdd(Rule *rule, string data, int type)
{
	string sql = "INSERT OR REPLACE INTO " TABLE_NAME " (rule_id, data, type, enable, rule_addr) VALUES ('" + rule->GetId() + "','" + macaron::Base64::Encode(data) + "'," + to_string(type) + ", " + to_string(rule->GetStatus()) + ", " + to_string(rule->GetAddr()) + ");";
	return Sqlite_Exec(sql);
}

int Db::RuleUpdateData(Rule *rule, string data)
{
	string sql = "UPDATE " TABLE_NAME " SET data='" + data + "' WHERE rule_id='" + rule->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::RuleUpdateStatus(Rule *rule)
{
	string sql = "UPDATE " TABLE_NAME " SET enable=" + to_string(rule->GetStatus()) + " WHERE rule_id='" + rule->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::RuleUpdateType(Rule *rule, int type)
{
	string sql = "UPDATE " TABLE_NAME " SET type=" + to_string(type) + " WHERE rule_id='" + rule->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::RuleUpdateAddr(Rule *rule)
{
	string sql = "UPDATE " TABLE_NAME " SET rule_addr=" + to_string(rule->GetAddr()) + " WHERE rule_id='" + rule->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::RuleDel(Rule *rule)
{
	string sql = "DELETE FROM " TABLE_NAME " WHERE rule_id='" + rule->GetId() + "';";
	return Sqlite_Exec(sql);
}

int Db::RuleDelAll()
{
	string sql = "DELETE FROM " TABLE_NAME ";";
	return Sqlite_Exec(sql);
}
