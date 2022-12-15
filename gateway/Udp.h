#pragma once

#include <thread>
#include <functional>
#include "json.h"
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

typedef function<int(Json::Value &reqValue, Json::Value &respValue)> OnRPCCallbackFunc;

class Udp
{
private:
	map<string, OnRPCCallbackFunc> onRPCCallbackFuncList;

public:
	int fd;
	int port;
	bool isRunning;
	thread *udpThread;
	thread *udpBroadcastThread;

	Udp(int port);

	void init();
	void stop();

	/**
	 * @brief Send udp broadcast message to app when HC enters pairing mode
	 * 
	 */
	void StartUdpBroadcast();
	int UdpCmdCallbackRegister(string method, OnRPCCallbackFunc onRPCCallbackFunc);
	void UdpOnMessage(string message, struct sockaddr_in *si_other, int slen);
	int send(string message, struct sockaddr_in *si_other, int slen);
};
