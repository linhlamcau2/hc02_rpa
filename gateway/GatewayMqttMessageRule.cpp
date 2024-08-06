#include "Gateway.h"
#include "Log.h"
#include "Db.h"

void Gateway::InitMqttMessageRule()
{
	OnDeviceRpcCallbackRegister("createRule", bind(&Gateway::OnCreateRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("editRule", bind(&Gateway::OnEditRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("createRuleV2", bind(&Gateway::OnCreateRuleV2, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("editRuleV2", bind(&Gateway::OnEditRuleV2, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("delRule", bind(&Gateway::OnDeleteRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getRuleList", bind(&Gateway::OnGetRuleList, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getRuleInfo", bind(&Gateway::OnGetRuleInfo, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("activeRule", bind(&Gateway::OnActiveRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("actionRule", bind(&Gateway::OnActionRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("actionRuleCloud", bind(&Gateway::OnActionRuleCloud, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("addFavoriteRule", bind(&Gateway::OnAddFavoriteRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("delFavoriteRule", bind(&Gateway::OnDelFavoriteRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getFavoriteRule", bind(&Gateway::OnGetFavoriteRule, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("createRule", bind(&Gateway::OnCreateRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("editRule", bind(&Gateway::OnEditRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("createRuleV2", bind(&Gateway::OnCreateRuleV2, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("editRuleV2", bind(&Gateway::OnEditRuleV2, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("delRule", bind(&Gateway::OnDeleteRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getRuleList", bind(&Gateway::OnGetRuleList, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getRuleInfo", bind(&Gateway::OnGetRuleInfo, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("activeRule", bind(&Gateway::OnActiveRule, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("actionRule", bind(&Gateway::OnActionRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("addFavoriteRule", bind(&Gateway::OnAddFavoriteRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("delFavoriteRule", bind(&Gateway::OnDelFavoriteRule, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getFavoriteRule", bind(&Gateway::OnGetFavoriteRule, this, placeholders::_1, placeholders::_2));
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
		string cmdToHcapp = "editRule";
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
				Json::Value temp = Json::arrayValue;
				pushMsgHcCoreToHcApp("delScene", sceneBle->GetId(), sceneBle->GetName(), temp, "");
				delSceneBle(sceneBle);
				cmdToHcapp = "createRule";
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
			pushMsgHcCoreToHcApp(cmdToHcapp, rule->GetId(), rule->GetName(), deviceList, "");
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
			database->RuleUpdateStatus(rule);
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

int Gateway::OnActionRuleCloud(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnActionRuleCloud");
	if (reqValue.isMember("output") && reqValue["output"].isArray())
	{
		Json::Value outputList = reqValue["output"];
		for (auto &item : outputList)
		{
			if (item.isMember("deviceOutput") && item["deviceOutput"].isArray())
			{
				Json::Value deviceOutputList = item["deviceOutput"];
				for (auto &dev : deviceOutputList)
				{
					if (dev.isObject() && dev.isMember("id") && dev["id"].isString())
					{
						string idDev = dev["id"].asString();
						Device *device = getDeviceFromId(idDev);
						if (device)
						{
							device->DoJsonArray(dev);
						}
					}
				}
			}

			if (item.isMember("sceneOutput") && item["sceneOutput"].isArray())
			{
				Json::Value sceneOutputList = item["sceneOutput"];
				for (auto &sceneOutput : sceneOutputList)
				{
					if (sceneOutput.isObject() && sceneOutput.isMember("id") && sceneOutput["id"].isString())
					{
						string idScene = sceneOutput["id"].asString();
						SceneBle *sceneBle = getSceneBleFromId(idScene);
						if (sceneBle)
						{
							if (sceneOutput.isMember("delay") && sceneOutput["delay"].isInt())
							{
								int delay = sceneOutput["delay"].asInt();
								sleep(delay);
							}
							sceneBle->Do(false);
						}
					}
				}
			}
		}
	}
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "actionRuleCloudRsp";
	return CODE_OK;
}

int Gateway::OnAddFavoriteRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnAddFavoriteRule");
	int rs = CODE_OK;
	if (reqValue.isMember("rulelist") && reqValue["rulelist"].isArray())
	{
		Json::Value rules = reqValue["rulelist"];
		for (auto &item : rules)
		{
			if (item.isString())
			{
				string id = item.asString();
				Rule *rule = getRuleFromId(id);
				if (rule)
				{
					rule->SetIsFavorite(true);
					database->RuleUpdateFavorite(rule, true);
				}
				else
				{
					LOGW("Rule %s not found", id.c_str());
					rs = CODE_ERROR;
				}
			}
			else
			{
				LOGW("Is not string");
				rs = CODE_ERROR;
			}
		}
	}
	else
	{
		LOGW("Data error %s", reqValue.toString().c_str());
		rs = CODE_ERROR;
	}
	respValue["data"]["code"] = rs;
	respValue["cmd"] = "addFavoriteRuleRsp";
	return CODE_OK;
}

int Gateway::OnDelFavoriteRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnDelFavoriteRule");
	int rs = CODE_OK;
	if (reqValue.isMember("rulelist") && reqValue["rulelist"].isArray())
	{
		Json::Value scenes = reqValue["rulelist"];
		for (auto &temp : scenes)
		{
			if (temp.isString())
			{
				string id = temp.asString();
				Rule *rule = getRuleFromId(id);
				if (rule)
				{
					rule->SetIsFavorite(false);
					database->RuleUpdateFavorite(rule, false);
				}
				else
				{
					LOGW("Rule %s not found", id.c_str());
				}
			}
			else
			{
				LOGW("Is not string");
				rs = CODE_ERROR;
			}
		}
	}
	else
	{
		LOGW("Data error %s", reqValue.toString().c_str());
		rs = CODE_ERROR;
	}
	respValue["data"]["code"] = rs;
	respValue["cmd"] = "delFavoriteRuleRsp";
	return CODE_OK;
}

int Gateway::OnGetFavoriteRule(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetFavoriteRule");
	Json::Value list = Json::arrayValue;
	for (const auto &[id, rule] : ruleList)
	{
		if (rule->GetIsFavorite())
		{
			list.append(id);
		}
	}
	respValue["data"]["rules"] = list;
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "getFavoriteRuleRsp";
	return CODE_OK;
}