#pragma once

#include <stdint.h>
#include <string.h>

#define STRING_VALUE_MAX_SIZE 128
#define CONFIG_ENV "smh.@server[0]."

#define HOST_KEY "host"
#define HOST_DEFAULT "broker.hivemq.com"
#define PORT_KEY "port"
#define PORT_DEFAULT 1883
#define CLIENT_ID_KEY "client_id"
#define CLIENT_ID_DEFAULT "broker_online"
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
	string host;
	int port;
	string clientId;
	string username;
	string password;
	int keepAlive;

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
};

extern Config *config;
