#include <string>
#include <iostream>
#include <endian.h>
#include <fstream>
#include "Config.h"
#include "Log.h"
#include "json.h"
#include "Define.h"

#define TAG "Config"

Config *config = NULL;

static bool get_str_config_entry(Json::Value &jsonData, string key, string &value)
{
	if (jsonData.isObject() && jsonData.isMember(key) && jsonData[key].isString())
	{
		string temp = jsonData[key].asString();
		value = temp;
		return true;
	}
	else
	{
		LOGW("Json data error: %s", jsonData.toString().c_str());
		return false;
	}
}

static bool get_int_config_entry(Json::Value &jsonData, string key, int &value)
{
	if (jsonData.isObject() && jsonData.isMember(key) && jsonData[key].isInt())
	{
		int temp = jsonData[key].asInt();
		value = temp;
		return true;
	}
	else
	{
		LOGW("Json data error: %s", jsonData.toString().c_str());
		return false;
	}
}

static bool set_str_config_entry(Json::Value &jsonData, string key, string &value)
{
	if (jsonData.isObject() && jsonData.isMember(key) && jsonData[key].isString())
	{
		jsonData[key] = value;
		return true;
	}
	else
	{
		LOGW("Json data error: %s", jsonData.toString().c_str());
		return false;
	}
}

static bool set_int_config_entry(Json::Value jsonData, string key, int &value)
{
	if (jsonData.isObject() && jsonData.isMember(key) && jsonData[key].isInt())
	{
		jsonData[key] = value;
		return true;
	}
	else
	{
		LOGW("Json data error: %s", jsonData.toString().c_str());
		return false;
	}
}

static bool OpenFile(string file, Json::Value &jsonData)
{
	std::ifstream input(file.c_str());
	if (!input.is_open())
	{
		std::cerr << "Không thể mở tệp JSON." << std::endl;
		return false;
	}

	input >> jsonData;
	input.close();
	return true;
}

static bool Write2File(string file, Json::Value &jsonData)
{
	std::ofstream output(file.c_str());
	if (!output.is_open())
	{
		std::cerr << "Không thể mở tệp JSON." << std::endl;
		return false;
	}

	output << jsonData;
	output.close();
	return true;
}


Config::Config()
{
}

void Config::ReadConfig()
{
	char str_temp[STRING_VALUE_MAX_SIZE];
	int int_temp = 0;
	string value_temp;

	Json::Value jsonData;
	OpenFile(CONFIG_FILE_NAME, jsonData);

	if (get_str_config_entry(jsonData, HOST_KEY, value_temp))
		host = value_temp;
	else
		host = HOST_DEFAULT;

	if (get_int_config_entry(jsonData, PORT_KEY, int_temp))
		port = int_temp;
	else
		port = PORT_DEFAULT;

	if (get_str_config_entry(jsonData, CLIENT_ID_KEY, value_temp))
		clientId = value_temp;
	else
		clientId = CLIENT_ID_DEFAULT;

	if (get_str_config_entry(jsonData, USERNAME_KEY, value_temp))
		username = value_temp;
	else
		username = USERNAME_DEFAULT;

	if (get_str_config_entry(jsonData, PASSWORD_KEY, value_temp))
		password = value_temp;
	else
		password = PASSWORD_DEFAULT;

	if (get_int_config_entry(jsonData, KEEP_ALIVE_KEY, int_temp))
		keepAlive = int_temp;
	else
		keepAlive = KEEP_ALIVE_DEFAULT;

	if (get_str_config_entry(jsonData, MAC_KB9_KEY, value_temp))
		mac_kb9 = value_temp;
	else
		mac_kb9 = MAC_KB9_DEFAULT;

	Print();
}

void Config::Print()
{
	if (host == "")
		LOGE("Server NULL");
	LOGI("host: %s", host.c_str());
	LOGI("port: %d", port);
	LOGI("clientId: %s", clientId.c_str());
	LOGI("username: %s", username.c_str());
	LOGI("password: %s", password.c_str());
	LOGI("keepAlive: %d", keepAlive);

	if (localHost == "")
		LOGE("Local Null");
	LOGI("Local host: %s", localHost.c_str());
	LOGI("Local port: %d", localPort);
	LOGI("Local clientId: %s", localClientId.c_str());
	LOGI("Local username: %s", localUsername.c_str());
	LOGI("Local password: %s", localPassword.c_str());
	LOGI("Local keepAlive: %d", localKeepAlive);
}

// Get info server
string Config::GetHost()
{
	return host;
}

int Config::GetPort()
{
	return port;
}

string Config::GetClientId()
{
	return clientId;
}

string Config::GetUsername()
{
	return username;
}

string Config::GetPassword()
{
	return password;
}

int Config::GetKeepAlive()
{
	return keepAlive;
}

// Get info local
string Config::GetLocalHost()
{
	return localHost;
}

int Config::GetLocalPort()
{
	return localPort;
}

string Config::GetLocalClientId()
{
	return localClientId;
}

string Config::GetLocalUsername()
{
	return localUsername;
}

string Config::GetLocalPassword()
{
	return localPassword;
}

int Config::GetLocalKeepAlive()
{
	return localKeepAlive;
}

string Config::GetMacK9B()
{
	return mac_kb9;
}

bool Config::SetHost(string host)
{
	Json::Value jsonData;
	if (OpenFile(CONFIG_FILE_NAME, jsonData))
	{
		if (set_str_config_entry(jsonData, HOST_KEY, host))
		{
			Write2File(CONFIG_FILE_NAME, jsonData);
			return true;
		}
	}
	else
		LOGW("OpenFile failed");
	return false;
}

bool Config::SetPort(int port)
{
	Json::Value jsonData;
	if (OpenFile(CONFIG_FILE_NAME, jsonData))
	{
		if (set_int_config_entry(jsonData, PORT_KEY, port))
		{
			Write2File(CONFIG_FILE_NAME, jsonData);
			return true;
		}
	}
	else
		LOGW("OpenFile failed");
	return false;
}

bool Config::SetClientId(string clientId)
{
	Json::Value jsonData;
	if (OpenFile(CONFIG_FILE_NAME, jsonData))
	{
		if (set_str_config_entry(jsonData, CLIENT_ID_KEY, clientId))
		{
			Write2File(CONFIG_FILE_NAME, jsonData);
			return true;
		}
	}
	else
		LOGW("OpenFile failed");
	return false;
}

bool Config::SetUsername(string username)
{
	Json::Value jsonData;
	if (OpenFile(CONFIG_FILE_NAME, jsonData))
	{
		if (set_str_config_entry(jsonData, USERNAME_KEY, username))
		{
			Write2File(CONFIG_FILE_NAME, jsonData);
			return true;
		}
	}
	else
		LOGW("OpenFile failed");
	return false;
}

bool Config::SetPassword(string password)
{
	Json::Value jsonData;
	if (OpenFile(CONFIG_FILE_NAME, jsonData))
	{
		if (set_str_config_entry(jsonData, PASSWORD_KEY, password))
		{
			Write2File(CONFIG_FILE_NAME, jsonData);
			return true;
		}
	}
	else
		LOGW("OpenFile failed");
	return false;
}

bool Config::SetKeepAlive(int keepAlive)
{
	return true;
}

bool Config::SetMacK9B(string mac_kb9)
{
	Json::Value jsonData;
	if (OpenFile(CONFIG_FILE_NAME, jsonData))
	{
		if (set_str_config_entry(jsonData, MAC_KB9_KEY, mac_kb9))
		{
			Write2File(CONFIG_FILE_NAME, jsonData);
			return true;
		}
	}
	else
		LOGW("OpenFile failed");
	return false;
}
