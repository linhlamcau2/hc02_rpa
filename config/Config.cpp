#include <string>
#include <iostream>
#include <endian.h>
#include <uci.h>
#include "Config.h"
#include "Log.h"

#define TAG "Config"

Config *config = NULL;

/****************************************
 *                  API                 *
 ***************************************/
static bool get_str_config_entry(char *name, char *value)
{
	struct uci_context *ctx;
	struct uci_ptr ptr;
	char path[STRING_VALUE_MAX_SIZE];
	ctx = uci_alloc_context();
	snprintf(path, STRING_VALUE_MAX_SIZE, "%s", name);
	if ((uci_lookup_ptr(ctx, &ptr, path, true) != UCI_OK) || !ptr.o || !ptr.o->v.string)
	{
		//		uci_perror (ctx, "uci_lookup_ptr Error");
		uci_free_context(ctx);
		return false;
	}
	snprintf(value, STRING_VALUE_MAX_SIZE, "%s", ptr.o->v.string);
	uci_free_context(ctx);
	return true;
}

static bool get_int_config_entry(char *name, int *value)
{
	struct uci_context *ctx;
	struct uci_ptr ptr;
	char path[STRING_VALUE_MAX_SIZE];
	ctx = uci_alloc_context();
	snprintf(path, STRING_VALUE_MAX_SIZE, "%s", name);
	if ((uci_lookup_ptr(ctx, &ptr, path, true) != UCI_OK) || !ptr.o || !ptr.o->v.string)
	{
		//		uci_perror (ctx, "uci_lookup_ptr Error");
		uci_free_context(ctx);
		return false;
	}
	*value = atoi(ptr.o->v.string);
	uci_free_context(ctx);
	return true;
}

static bool set_str_config_entry(char *name, char *section_name, const char *value)
{
	struct uci_context *ctx;
	struct uci_ptr ptr;
	char path[STRING_VALUE_MAX_SIZE];
	ctx = uci_alloc_context();
	snprintf(path, STRING_VALUE_MAX_SIZE, "%s", name);
	if ((uci_lookup_ptr(ctx, &ptr, path, true) != UCI_OK))
	{
		uci_perror(ctx, "uci_lookup_ptr Error");
		uci_free_context(ctx);
		return false;
	}
	if (ptr.s == NULL)
	{
		if (uci_add_section(ctx, ptr.p, section_name, &ptr.s) != UCI_OK)
		{
			uci_perror(ctx, "UCI Error to add new section");
			uci_free_context(ctx);
			return false;
		}
	}
	ptr.option = section_name;
	ptr.value = value;
	if (uci_set(ctx, &ptr) != UCI_OK)
	{
		uci_perror(ctx, "UCI Error to set new option");
		uci_free_context(ctx);
		return false;
	}
	if (uci_commit(ctx, &ptr.p, false) != UCI_OK)
	{
		uci_perror(ctx, "UCI Error to commit changes");
		uci_free_context(ctx);
		return false;
	}
	uci_free_context(ctx);
	return true;
}

static bool set_int_config_entry(char *section, char *name, int value)
{
	struct uci_context *ctx;
	struct uci_ptr ptr;
	char strValue[20];
	ctx = uci_alloc_context();
	if ((uci_lookup_ptr(ctx, &ptr, section, true) != UCI_OK))
	{
		uci_perror(ctx, "uci_lookup_ptr Error");
		uci_free_context(ctx);
		return false;
	}
	if (ptr.s == NULL)
	{
		if (uci_add_section(ctx, ptr.p, "device", &ptr.s) != UCI_OK)
		{
			uci_perror(ctx, "UCI Error to add new section");
			uci_free_context(ctx);
			return false;
		}
	}
	snprintf(strValue, 20, "%d", value);
	ptr.option = name;
	ptr.value = strValue;
	if (uci_set(ctx, &ptr) != UCI_OK)
	{
		uci_perror(ctx, "UCI Error to set new option");
		uci_free_context(ctx);
		return false;
	}
	if (uci_commit(ctx, &ptr.p, false) != UCI_OK)
	{
		uci_perror(ctx, "UCI Error to commit changes");
		uci_free_context(ctx);
		return false;
	}
	uci_free_context(ctx);
	return true;
}

// static bool delete_section(char *section)
// {
// 	struct uci_context *ctx;
// 	struct uci_ptr ptr;
// 	ctx = uci_alloc_context();
// 	if ((uci_lookup_ptr(ctx, &ptr, section, true) != UCI_OK))
// 	{
// 		uci_perror(ctx, "uci_lookup_ptr Error");
// 		uci_free_context(ctx);
// 		return false;
// 	}
// 	if (ptr.s != NULL)
// 	{
// 		if (uci_delete(ctx, &ptr) != UCI_OK)
// 		{
// 			uci_perror(ctx, "UCI Error to delete section");
// 			uci_free_context(ctx);
// 			return false;
// 		}
// 	}
// 	if (uci_commit(ctx, &ptr.p, false) != UCI_OK)
// 	{
// 		uci_perror(ctx, "UCI Error to commit changes");
// 		uci_free_context(ctx);
// 		return false;
// 	}
// 	uci_free_context(ctx);
// 	return true;
// }

Config::Config()
{
}

void Config::ReadConfig()
{
	char str_temp[STRING_VALUE_MAX_SIZE];
	int int_temp = 0;

	// server
	if (get_str_config_entry((char *)CONFIG_ENV HOST_KEY, str_temp))
		host = string(str_temp);
	else
		host = HOST_DEFAULT;

	if (get_int_config_entry((char *)CONFIG_ENV PORT_KEY, &int_temp))
		port = int_temp;
	else
		port = PORT_DEFAULT;

	if (get_str_config_entry((char *)CONFIG_ENV CLIENT_ID_KEY, str_temp))
		clientId = string(str_temp);
	else
		clientId = CLIENT_ID_DEFAULT;

	if (get_str_config_entry((char *)CONFIG_ENV USERNAME_KEY, str_temp))
		username = string(str_temp);
	else
		username = USERNAME_DEFAULT;

	if (get_str_config_entry((char *)CONFIG_ENV PASSWORD_KEY, str_temp))
		password = string(str_temp);
	else
		password = PASSWORD_DEFAULT;

	if (get_int_config_entry((char *)CONFIG_ENV KEEP_ALIVE_KEY, &int_temp))
		keepAlive = int_temp;
	else
		keepAlive = KEEP_ALIVE_DEFAULT;

	// local

	if (get_str_config_entry((char *)CONFIG_ENV_LOCAL HOST_KEY, str_temp))
		localHost = string(str_temp);
	else
		localHost = HOST_DEFAULT;

	if (get_int_config_entry((char *)CONFIG_ENV_LOCAL PORT_KEY, &int_temp))
		localPort = int_temp;
	else
		localPort = PORT_DEFAULT;

	if (get_str_config_entry((char *)CONFIG_ENV_LOCAL CLIENT_ID_KEY, str_temp))
		localClientId = string(str_temp);
	else
		localClientId = CLIENT_ID_DEFAULT;

	if (get_str_config_entry((char *)CONFIG_ENV_LOCAL USERNAME_KEY, str_temp))
		localUsername = string(str_temp);
	else
		localUsername = USERNAME_DEFAULT;

	if (get_str_config_entry((char *)CONFIG_ENV_LOCAL PASSWORD_KEY, str_temp))
		localPassword = string(str_temp);
	else
		localPassword = PASSWORD_DEFAULT;

	if (get_int_config_entry((char *)CONFIG_ENV_LOCAL KEEP_ALIVE_KEY, &int_temp))
		localKeepAlive = int_temp;
	else
		localKeepAlive = KEEP_ALIVE_DEFAULT;

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

bool Config::SetHost(string host)
{
	if (set_str_config_entry((char *)CONFIG_ENV HOST_KEY, HOST_KEY, host.c_str()))
	{
		return true;
	}
	return false;
}
bool Config::SetPort(int port)
{
	if (set_int_config_entry((char *)CONFIG_ENV PORT_KEY, PORT_KEY, port))
	{
		return true;
	}
	return false;
}
bool Config::SetClientId(string clientId)
{
	if (set_str_config_entry((char *)CONFIG_ENV CLIENT_ID_KEY, CLIENT_ID_KEY, clientId.c_str()))
	{
		return true;
	}
	return false;
}
bool Config::SetUsername(string username)
{
	if (set_str_config_entry((char *)CONFIG_ENV USERNAME_KEY, USERNAME_KEY, username.c_str()))
	{
		return true;
	}
	return false;
}
bool Config::SetPassword(string password)
{
	if (set_str_config_entry((char *)CONFIG_ENV PASSWORD_KEY, PASSWORD_KEY, password.c_str()))
	{
		return true;
	}
	return false;
}
bool Config::SetKeepAlive(int keepAlive)
{
}
