#include "Gateway.h"
#include "Log.h"
#include "Wifi.h"
#include "Base64.h"
#include <fstream>
#include <string.h>

void Gateway::InitMqttMessageHc()
{
	OnDeviceRpcCallbackRegister("controlHc", bind(&Gateway::OnControlHc, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("getHcInfo", bind(&Gateway::OnGetHcInfo, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("startScanBle", bind(&Gateway::OnStartScanBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("stopScanBle", bind(&Gateway::OnStopScanBle, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("resetHc", bind(&Gateway::OnResetHC, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("versionHc", bind(&Gateway::OnVersionHC, this, placeholders::_1, placeholders::_2));
	OnDeviceRpcCallbackRegister("SSHRemote", bind(&Gateway::OnSSHRemote, this, placeholders::_1, placeholders::_2));

	OnLocalCallbackRegister("controlHc", bind(&Gateway::OnControlHc, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("getHcInfo", bind(&Gateway::OnGetHcInfo, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("startScanBle", bind(&Gateway::OnStartScanBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("stopScanBle", bind(&Gateway::OnStopScanBle, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("resetHc", bind(&Gateway::OnResetHC, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("versionHc", bind(&Gateway::OnVersionHC, this, placeholders::_1, placeholders::_2));
	OnLocalCallbackRegister("SSHRemote", bind(&Gateway::OnSSHRemote, this, placeholders::_1, placeholders::_2));
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
	respValue["cmd"] = "startScanBleRsp";
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
	respValue["cmd"] = "stopScanBleRsp";
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

int Gateway::OnSSHRemote(Json::Value &reqValue, Json::Value &respValue)
{
	int rs = CODE_OK;
	if (reqValue.isMember("type") && reqValue["type"].isString() &&
			reqValue.isMember("key") && reqValue["key"].isString() &&
			reqValue.isMember("user") && reqValue["user"].isString() &&
			reqValue.isMember("host") && reqValue["host"].isString() &&
			reqValue.isMember("serverPort") && reqValue["serverPort"].isInt() &&
			reqValue.isMember("forwardPort") && reqValue["forwardPort"].isInt())
	{
		string key = "";
		string type = reqValue["type"].asString();
		string user = reqValue["user"].asString();
		string host = reqValue["host"].asString();
		uint32_t serverPort = reqValue["serverPort"].asInt();
		uint32_t forwardPort = reqValue["forwardPort"].asInt();
		uint32_t localPort = 22;
		if (reqValue.isMember("localPort") && reqValue["localPort"].isInt())
		{
			localPort = reqValue["localPort"].asInt();
		}
		if (type == "base64")
		{
			string keyBase64 = reqValue["key"].asString();
			string decode = macaron::Base64::Decode(keyBase64, key);
			if (decode != "")
			{
				rs = CODE_ERROR;
				LOGW("Base64 decode err: %s", decode.c_str());
			}
		}
		else
		{
			key = reqValue["key"].asString();
		}

		if (rs == 0)
		{
			// save key file
			system("rm /key.txt");
			system("rm /output.txt");
			ofstream keyFile("/key.txt");
			keyFile << key;
			keyFile.close();

			system("chmod 600 /key.txt");
			system("killall ssh");
			string cmd = "ssh -i /key.txt -o StrictHostKeyChecking=no -f -N -T -R" + to_string(forwardPort) + ":localhost:" + to_string(localPort) + " " + user + "@" + host + " -p " + to_string(serverPort);
			cmd += " >> /output.txt 2>&1";
			LOGI("cmd: %s", cmd.c_str());
			system(cmd.c_str());
			sleep(2);
			FILE *fp = fopen("/output.txt", "r");
			char path[512] = {0};
			if (fp)
			{
				while (fgets(path, sizeof(path), fp) != NULL)
				{
					if (strlen(path) > 1)
					{
						LOGW("SSH err: %s", path);
						rs = CODE_ERROR;
						break;
					}
				}
				fclose(fp);
			}
		}
	}
	else
	{
		rs = CODE_FORMAT_ERROR;
	}
	respValue["code"] = rs;
	respValue["cmd"] = "SSHRemoteRsp";
	return CODE_OK;
}
