#pragma once

#include <stdint.h>
#include <string.h>
#include <vector>
#define STRING_VALUE_MAX_SIZE 128
// #define CONFIG_ENV "smh.@server[0]."
// #define CONFIG_ENV_LOCAL "smh.@local[0]."

#define HOST_KEY "host"
#define HOST_DEFAULT "210.211.96.132"
#define PORT_KEY "port"
#define PORT_DEFAULT 1884
#define CLIENT_ID_KEY "client_id"
#define CLIENT_ID_DEFAULT "hc"
#define USERNAME_KEY "username"
#define USERNAME_DEFAULT "hc"
#define PASSWORD_KEY "password"
#define PASSWORD_DEFAULT "ABC123456"
#define KEEP_ALIVE_KEY "keep_alive"
#define KEEP_ALIVE_DEFAULT 10
#define MAC_KB9_KEY "mac_kb9"
#define MAC_KB9_DEFAULT ""

#define ADDR_CTCU "addr_ctcu"
#define ADDR_CTCU_DEFAULT ""

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
	string mac_kb9;
	vector<int> ctcu_addr;
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
	string GetMacK9B();
	vector<int> GetCtcudAddr();

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
	bool SetMacK9B(string mac_kb9);
};

extern Config *config;
