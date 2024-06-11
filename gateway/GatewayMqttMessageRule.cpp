#include "Gateway.h"
#include "Log.h"
#include "Db.h"

void Gateway::InitMqttMessageRule()
{
	// OnDeviceRpcCallbackRegister("createRule", bind(&Gateway::OnCreateRule, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("editRule", bind(&Gateway::OnEditRule, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("createRuleV2", bind(&Gateway::OnCreateRuleV2, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("editRuleV2", bind(&Gateway::OnEditRuleV2, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("delRule", bind(&Gateway::OnDeleteRule, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("getRuleList", bind(&Gateway::OnGetRuleList, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("getRuleInfo", bind(&Gateway::OnGetRuleInfo, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("activeRule", bind(&Gateway::OnActiveRule, this, placeholders::_1, placeholders::_2));
	// OnDeviceRpcCallbackRegister("actionRule", bind(&Gateway::OnActionRule, this, placeholders::_1, placeholders::_2));

	// OnLocalCallbackRegister("createRule", bind(&Gateway::OnCreateRule, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("editRule", bind(&Gateway::OnEditRule, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("createRuleV2", bind(&Gateway::OnCreateRuleV2, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("editRuleV2", bind(&Gateway::OnEditRuleV2, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("delRule", bind(&Gateway::OnDeleteRule, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("getRuleList", bind(&Gateway::OnGetRuleList, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("getRuleInfo", bind(&Gateway::OnGetRuleInfo, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("activeRule", bind(&Gateway::OnActiveRule, this, placeholders::_1, placeholders::_2));
	// OnLocalCallbackRegister("actionRule", bind(&Gateway::OnActionRule, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnGetRuleList(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetRuleList");
	Json::Value ruleData = Json::arrayValue;
	ruleListMtx.lock();
	for (const auto &[id, rule] : ruleList)
	{
		Json::Value ruleValue;
		ruleValue["id"] = rule->GetId();
		ruleValue["name"] = rule->GetName();
		ruleData.append(ruleValue);
	}
	ruleListMtx.unlock();
	respValue["data"]["rules"] = ruleData;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getRuleListRsp";
	return CODE_OK;
}

int Gateway::OnGetRuleInfo(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnOnGetRuleInfo");
	if (reqValue.isMember("rules") && reqValue["rules"].isArray() && reqValue["rules"].size() > 0)
	{
		Json::Value rules = reqValue["rules"];
		Json::Value ruleData = Json::arrayValue;
		for (auto &ruleValue : rules)
		{
			if (ruleValue.isString())
			{
				string ruleId = ruleValue.asString();
				Rule *temp_rule = getRuleFromId(ruleId);
				if (temp_rule)
				{
					ruleData.append(temp_rule->GetRuleData());
					respValue["data"]["code"] = CODE_OK;
				}
			}
			else
				respValue["data"]["code"] = CODE_FORMAT_ERROR;
		}
		respValue["data"]["rules"] = ruleData;
	}
	respValue["cmd"] = "getRuleInfoRsp";
	return CODE_OK;
}

int Gateway::OnCreateRule(Json::Value &reqValue, Json::Value &respValue)
{
	Rule *rule = AddRule(reqValue, true);
	if (rule)
	{
		LOGI("Add Rule %s", rule->GetId().c_str());
		Json::Value deviceList = Json::arrayValue;
		pushMsgHcCoreToHcApp("createRule", rule->GetId(), rule->GetName(), deviceList, "");
		// rule->Check();
		respValue["data"]["code"] = CODE_OK;
		respValue["data"]["id"] = rule->GetId();
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "createRuleRsp";
	return CODE_OK;
}

int Gateway::OnEditRule(Json::Value &reqValue, Json::Value &respValue)
{
	int rs = CODE_ERROR;
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string ruleId = reqValue["id"].asString();
		Rule *rule = getRuleFromId(ruleId);
		if (!rule)
		{
			SceneBle *sceneBle = getSceneBleFromId(ruleId);
			if (sceneBle)
			{
				vector<DeviceInSceneBle *> devicesInSceneBle = sceneBle->deviceList;
				for (auto &deviceInScene : devicesInSceneBle)
				{
					sceneBle->DelDevice(deviceInScene->device, true, true);
					
				}
				delSceneBle(sceneBle);
			}
		}
		if (rule)
		{
			delRule(rule);
		}
		rule = AddRule(reqValue, true);
		if (rule)
		{
			LOGI("Edit Rule %s", rule->GetId().c_str());
			Json::Value deviceList = Json::arrayValue;
			pushMsgHcCoreToHcApp("editRule", rule->GetId(), rule->GetName(), deviceList, "");
			// rule->Check();
			rs = CODE_OK;
		}
		else
		{
			rs = CODE_FORMAT_ERROR;
		}
		respValue["data"]["id"] = ruleId;
	}
	else
	{
		rs = CODE_FORMAT_ERROR;
	}
	respValue["data"]["code"] = rs;
	respValue["cmd"] = "editRuleRsp";
	return CODE_OK;
}

int Gateway::OnCreateRuleV2(Json::Value &reqValue, Json::Value &respValue)
{
	Rule *rule = AddRuleV2(reqValue, true);
	if (rule)
	{
		LOGI("Add Rule v2 %s", rule->GetId().c_str());
		Json::Value deviceList = Json::arrayValue;
		pushMsgHcCoreToHcApp("createRule", rule->GetId(), rule->GetName(), deviceList, "");
		// rule->Check();
		respValue["data"]["code"] = CODE_OK;
		respValue["data"]["id"] = rule->GetId();
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "createRuleRsp";
	return CODE_OK;
}

int Gateway::OnEditRuleV2(Json::Value &reqValue, Json::Value &respValue)
{
	int rs = CODE_ERROR;
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string ruleId = reqValue["id"].asString();
		Rule *rule = getRuleFromId(ruleId);
		if (!rule)
		{
			SceneBle *sceneBle = getSceneBleFromId(ruleId);
			if (sceneBle)
			{
				vector<DeviceInSceneBle *> devicesInSceneBle = sceneBle->deviceList;
				for (auto &deviceInScene : devicesInSceneBle)
				{
					sceneBle->DelDevice(deviceInScene->device, true, true);
					
				}
				delSceneBle(sceneBle);
			}
		}
		if (rule)
		{
			delRule(rule);
		}
		rule = AddRuleV2(reqValue, true);
		if (rule)
		{
			LOGI("Edit Rule v2 %s", rule->GetId().c_str());
			Json::Value deviceList = Json::arrayValue;
			pushMsgHcCoreToHcApp("editRule", rule->GetId(), rule->GetName(), deviceList, "");
			// rule->Check();
			rs = CODE_OK;
		}
		else
		{
			rs = CODE_FORMAT_ERROR;
		}
		respValue["data"]["id"] = ruleId;
	}
	else
	{
		rs = CODE_FORMAT_ERROR;
	}
	respValue["data"]["code"] = rs;
	respValue["cmd"] = "editRuleRsp";
	return CODE_OK;
}

int Gateway::OnDeleteRule(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string ruleId = reqValue["id"].asString();
		Rule *rule = getRuleFromId(ruleId);
		if (rule)
		{
			Json::Value deviceList = Json::arrayValue;
			pushMsgHcCoreToHcApp("delRule", rule->GetId(), rule->GetName(), deviceList, "");
			delRule(rule);
			respValue["data"]["code"] = CODE_OK;
		}
		else
		{
			respValue["data"]["code"] = CODE_NOT_FOUND_RULE;
		}
		respValue["data"]["id"] = ruleId;
	}
	else
	{
		respValue["data"]["code"] = CODE_FORMAT_ERROR;
	}
	respValue["cmd"] = "delRuleRsp";
	return CODE_OK;
}

int Gateway::OnActiveRule(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString() &&
		reqValue.isMember("status") && reqValue["status"].isInt())
	{
		string id = reqValue["id"].asString();
		int status = reqValue["status"].asInt();
		Rule *rule = getRuleFromId(id);
		if (rule)
		{
			rule->SetStatus(status);
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "activeRuleRsp";
	return CODE_OK;
}

int Gateway::OnActionRule(Json::Value &reqValue, Json::Value &respValue)
{
	if (reqValue.isMember("id") && reqValue["id"].isString())
	{
		string id = reqValue["id"].asString();
		Rule *rule = getRuleFromId(id);
		if (rule)
		{
			rule->RunOutput();
			respValue["data"]["id"] = rule->GetId();
			Json::Value deviceList = Json::arrayValue;
			pushMsgHcCoreToHcApp("actionRuleRsp", rule->GetId(), rule->GetName(), deviceList, "");
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "actionRuleRsp";
	return CODE_OK;
}
