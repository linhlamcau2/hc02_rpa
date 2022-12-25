#pragma once

#include <thread>
#include <functional>
#include "json.h"
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

class Udp
{
private:
	typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnRPCCallbackFunc;
	map<string, OnRPCCallbackFunc> onRPCCallbackFuncList;

public:
	int fd;
	int port;
	bool isRunning;
	thread *udpThread;

	Udp(int port);

	void init();
	void stop();

	int UdpCmdCallbackRegister(string method, OnRPCCallbackFunc onRPCCallbackFunc);
	void UdpOnMessage(string message, struct sockaddr_in *si_other, int slen);
	int send(string message, struct sockaddr_in *si_other, int slen);
};
