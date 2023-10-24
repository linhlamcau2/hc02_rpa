#include "Gateway.h"
#include "Log.h"
#include "Wifi.h"
#include "Base64.h"
#include "Config.h"
#include <fstream>
#include <string.h>
#include "Util.h"

void Gateway::InitMqttMessageHc()
{
	OnDeviceRpcCallbackRegister("controlHc", bind(&Gateway::OnControlHc, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getHcInfo", bind(&Gateway::OnGetHcInfo, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("startScanBle", bind(&Gateway::OnStartScanBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("stopScanBle", bind(&Gateway::OnStopScanBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("resetHc", bind(&Gateway::OnResetHC, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("versionHc", bind(&Gateway::OnVersionHC, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("CreateTunnel", bind(&Gateway::OnCreateTunnel, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("DeleteAllTunnel", bind(&Gateway::OnDeleteAllTunnel, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("controlHc", bind(&Gateway::OnControlHc, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getHcInfo", bind(&Gateway::OnGetHcInfo, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("startScanBle", bind(&Gateway::OnStartScanBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("stopScanBle", bind(&Gateway::OnStopScanBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("resetHc", bind(&Gateway::OnResetHC, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("versionHc", bind(&Gateway::OnVersionHC, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("otaHC", bind(&Gateway::OnOtaHc, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("setPasswordMqtt", bind(&Gateway::OnSetPasswordMqtt, this, placeholders::_1, placeholders::_2));
}

int Gateway::OnControlHc(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnControlHc");
	int rs = Do(reqValue);
	respValue["data"]["code"] = rs;
	respValue["cmd"] = "controlGwRsp";
	return CODE_OK;
}

int Gateway::OnGetHcInfo(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("OnGetHcInfo");
	Json::Value dataValue;
	dataValue["mac"] = mac;
	dataValue["ip"] = Wifi::GetIP();
	dataValue["isConnectCloud"] = CloudProtocol::isConnected();
	dataValue["name"] = "RD HC";
	dataValue["type"] = MODEL;
	dataValue["ver"] = STR(VERSION);
	respValue["data"] = dataValue;
	respValue["cmd"] = "getHcInfoRsp";
	return CODE_OK;
}

int Gateway::OnStartScanBle(Json::Value &reqValue, Json::Value &respValue)
{
	int rsCode = CODE_OK;
	if (bleProtocol)
	{
		bleProtocol->SetProvisioning(true);
		bleProtocol->StartScan();
	}
	else
	{
		rsCode = CODE_ERROR;
		LOGW("BleProtocol null");
	}

#ifdef CONFIG_ENABLE_ZIGBEE
	if (zigbeeProtocol)
	{
		zigbeeProtocol->PermitJoin(120);
	}
	else
	{
		rsCode = CODE_ERROR;
		LOGW("ZigbeeProtocol null");
	}
#endif

	respValue["data"]["code"] = rsCode;
	respValue["cmd"] = "startScanBle";
	return CODE_OK;
}

int Gateway::OnStopScanBle(Json::Value &reqValue, Json::Value &respValue)
{
	int rsCode = CODE_OK;
	if (bleProtocol)
	{
		bleProtocol->StopScan();
	}
	else
	{
		rsCode = CODE_ERROR;
		LOGW("BleProtocol null");
	}

#ifdef CONFIG_ENABLE_ZIGBEE
	if (zigbeeProtocol)
	{
		zigbeeProtocol->PermitJoin(0);
	}
	else
	{
		rsCode = CODE_ERROR;
		LOGW("ZigbeeProtocol null");
	}
#endif

	respValue["data"]["code"] = rsCode;
	respValue["cmd"] = "stopScanBle";
	return CODE_OK;
}

int Gateway::OnResetHC(Json::Value &reqValue, Json::Value &respValue)
{
	LOGW("OnResetFactory");
	ResetFactory();
	respValue["data"]["code"] = CODE_OK;
	respValue["cmd"] = "resetHcRsp";
	return CODE_EXIT;
}

int Gateway::OnVersionHC(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("Version HC");
	respValue["data"]["code"] = CODE_OK;
	respValue["data"]["version"] = STR(VERSION);
	respValue["cmd"] = "versionHcRsp";
	return CODE_OK;
}

int Gateway::OnCreateTunnel(Json::Value &reqValue, Json::Value &respValue)
{
	int err = 0;
	LOGD("Create Tunnel");
	if (reqValue.isMember("params") && reqValue["params"].isObject())
	{
		Json::Value params = reqValue["params"];
		if (params.isMember("type") && params["type"].isString() &&
				params.isMember("key") && params["key"].isString() &&
				params.isMember("user") && params["user"].isString() &&
				params.isMember("host") && params["host"].isString() &&
				params.isMember("serverPort") && params["serverPort"].isInt() &&
				params.isMember("forwardPort") && params["forwardPort"].isInt())
		{
			string key = "";
			string type = params["type"].asString();
			string user = params["user"].asString();
			string host = params["host"].asString();
			uint32_t serverPort = params["serverPort"].asInt();
			uint32_t forwardPort = params["forwardPort"].asInt();

			uint32_t localPort = 22;
			if (params.isMember("localPort") && params["localPort"].isInt())
			{
				localPort = params["localPort"].asInt();
			}
			LOGI("Create Tunnel %d --> %s:%d", localPort, host.c_str(), forwardPort);
			if (type == "base64")
			{
				string keyBase64 = params["key"].asString();
				string decode = macaron::Base64::Decode(keyBase64, key);
				if (decode != "")
				{
					err = 1;
					LOGW("Base64 decode err: %s", decode.c_str());
				}
			}
			else
			{
				key = params["key"].asString();
			}

			if (err == 0)
			{
				// save key file
				system("rm " TMP_FOLDER "key.txt");
				system("rm " TMP_FOLDER "output.txt");
				ofstream keyFile(TMP_FOLDER "key.txt");
				keyFile << key;
				keyFile.close();

				system("chmod 600 " TMP_FOLDER "key.txt");
				// system("killall ssh");
				string cmd = "ssh -i " TMP_FOLDER "key.txt -o StrictHostKeyChecking=no -f -N -T -R" + to_string(forwardPort) + ":localhost:" + to_string(localPort) + " " + user + "@" + host + " -p " + to_string(serverPort);
				cmd += " >> " TMP_FOLDER "output.txt 2>&1";
				LOGI("cmd: %s", cmd.c_str());
				system(cmd.c_str());
				sleep(2);
				bool err = false;
				FILE *fp = fopen("" TMP_FOLDER "output.txt", "r");
				char path[512] = {0};
				if (fp)
				{
					while (fgets(path, sizeof(path), fp) != NULL)
					{
						if (strlen(path) > 1)
						{
							LOGW("SSH err: %s", path);
							err = true;
							break;
						}
					}
					fclose(fp);
				}
				if (err)
				{
					respValue["msg"] = string(path);
					respValue["code"] = 1;
				}
				else
				{
					respValue["code"] = 0;
				}
				return CODE_OK;
			}
		}
		else
		{
			LOGE("Error data");
			respValue["data"]["code"] = CODE_FORMAT_ERROR;
		}
	}
	else
	{
		LOGW("Error");
	}

	respValue["code"] = err;
	return CODE_OK;
}

int Gateway::OnDeleteAllTunnel(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("Delete all Tunnel");
	system("killall ssh");
	respValue["code"] = 0;
	return CODE_OK;
}

int Gateway::OnOtaHc(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("Create Tunnel");
	if (reqValue.isMember("url") && reqValue["url"].isString() && reqValue.isMember("checkSum") && reqValue["checkSum"].isString())
	{
		string url = reqValue["url"].asString();
		string sha = reqValue["checkSum"].asString();
		string cmd = "wget -P " TMP_FOLDER " "+ url;
		system(cmd.c_str());
		if(Util::CheckSHA256(TMP_FOLDER "rd.tar.gz", sha) != CODE_OK)
		{
			cmd = "rm " TMP_FOLDER "rd.tar.gz";
			system(cmd.c_str());
		}
		else
		{
			cmd = "tar -xzf " TMP_FOLDER "rd.tar.gz -C " TMP_FOLDER;
			system(cmd.c_str());
			cmd = "./" TMP_FOLDER "config.sh";
			system(cmd.c_str());
		}
		return CODE_OK;
	}
	return CODE_ERROR;
}

/*
{
  "cmd": "setPasswordMqtt",
  "rqi": "abc-xyz-mnl",
  "data": {
	"password": "ABC123456"
  }
}
*/

int Gateway::OnSetPasswordMqtt(Json::Value &reqValue, Json::Value &respValue)
{
	LOGD("Set password mqtt");
	if (reqValue.isMember("password") && reqValue["password"].isString())
	{
		string password = reqValue["password"].asString();
		string client_id = "hc-" + mac;
		string username = "hc-" + mac;
		if (config->SetHost("mqtt.rangdong.com.vn"))
		{
			if (config->SetPort(8883))
			{
				if (config->SetClientId(client_id))
				{
					if (config->SetUsername(username))
					{
						if (config->SetPassword(password))
						{
							return CODE_EXIT;
						}
						else
							LOGW("set password error");
					}
					else
						LOGW("set username error");
				}
				else
					LOGW("set client error");
			}
			else
				LOGW("set port error");
		}
		else
			LOGW("set host error");
	}
	else
		LOGW("format error: %s", reqValue.toString().c_str());
	return CODE_ERROR;
}
