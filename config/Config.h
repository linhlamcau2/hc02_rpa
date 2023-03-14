#pragma once

#include <stdint.h>
#include <string.h>

#define STRING_VALUE_MAX_SIZE 128
#define CONFIG_ENV "smh.@server[0]."
#define CONFIG_ENV_LOCAL "smh.@local[0]."

#define HOST_KEY "host"
#define HOST_DEFAULT "mqtt.truesight.asia"
#define PORT_KEY "port"
#define PORT_DEFAULT 1883
#define CLIENT_ID_KEY "client_id"
#define CLIENT_ID_DEFAULT ""
#define USERNAME_KEY "username"
#define USERNAME_DEFAULT ""
#define PASSWORD_KEY "password"
#define PASSWORD_DEFAULT ""
#define KEEP_ALIVE_KEY "keep_alive"
#define KEEP_ALIVE_DEFAULT 10

using namespace std;

class Config
{
private:
	// server
	string host;
	int port;
	string clientId;
	string username;
	string password;
	int keepAlive;

	// local
	string localHost;
	int localPort;
	string localClientId;
	string localUsername;
	string localPassword;
	int localKeepAlive;

public:
	Config();

	void ReadConfig();
	void Print();

	string GetHost();
	int GetPort();
	string GetClientId();
	string GetUsername();
	string GetPassword();
	int GetKeepAlive();

	string GetLocalHost();
	int GetLocalPort();
	string GetLocalClientId();
	string GetLocalUsername();
	string GetLocalPassword();
	int GetLocalKeepAlive();

	bool SetHost(string host);
	bool SetPort(int port);
	bool SetClientId(string clientId);
	bool SetUsername(string username);
	bool SetPassword(string password);
	bool SetKeepAlive(int keepAlive);

	bool SetLocalHost(string localHost);
	bool SetLocalPort(int localPort);
	bool SetLocalClientId(string localClientId);
	bool SetLocalUsername(string localUsername);
	bool SetLocalPassword(string localPassword);
	bool SetLocalKeepAlive(int keepAlive);
};

extern Config *config;
