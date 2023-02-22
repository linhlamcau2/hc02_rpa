#include <string>
#include <iostream>
#include <endian.h>

#ifndef ANDROID
#include <uci.h>
#endif

#include "Config.h"
#include "Log.h"

#define TAG "Config"

							 Config *config = NULL;

/****************************************
 *                  API                 *
 ***************************************/
static bool get_str_config_entry(char *name, char *value)
{
#ifndef ANDROID
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
#else
	return false;
#endif
}

static bool get_int_config_entry(char *name, int *value)
{
#ifndef ANDROID
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
#else
	return false;
#endif
}

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
