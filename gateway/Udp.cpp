#include "Udp.h"
#include "Log.h"

#include <stdio.h>	//printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <unistd.h>
#include "Base64.h"

#include "Util.h"
#include "Wifi.h"
#include <thread>

#define BUFLEN 1024

Udp::Udp(int port) : port(port)
{
	isRunning = false;
}

static void UdpHandleMessage(void *data)
{
	LOGI("Start UdpHandleMessage");
	Udp *udp = (Udp *)data;
	struct sockaddr_in si_me, si_other;
	int slen = sizeof(si_other);

	int recv_len;
	char buf[BUFLEN];

	// create a UDP socket
	if ((udp->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		LOGE("UDP die");
		exit(1);
	}

	int broadcastEnable = 1;
	setsockopt(udp->fd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

	// zero out the structure
	memset((char *)&si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(udp->port);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind socket to port
	if (::bind(udp->fd, (struct sockaddr *)&si_me, sizeof(si_me)) == -1)
	{
		LOGE("UDP die");
		exit(1);
	}

	// keep listening for data
	while (udp->isRunning)
	{
		LOGI("UDP Waiting for data...");
		fflush(stdout);

		// try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(udp->fd, buf, BUFLEN, 0, (struct sockaddr *)&si_other, (socklen_t *)&slen)) == -1)
		{
			LOGE("UDP die: recvfrom");
			exit(1);
		}

		buf[recv_len] = '\0';
		udp->UdpOnMessage(string(buf), &si_other, slen);
	}
#ifdef ESP_PLATFORM
	vTaskDelete(NULL);
#endif
}

void Udp::init()
{
	if (!isRunning)
	{
		isRunning = true;
#ifdef ESP_PLATFORM
		if (xTaskCreate(UdpHandleMessage, "UdpHandleMessage", 10240, this, 10, NULL) != pdPASS)
			LOGE("Failed to create task\n");
		vTaskDelay(10);
#else
		thread udpThread(UdpHandleMessage, this);
		udpThread.detach();
#endif
	}
}

void Udp::stop()
{
	isRunning = false;
}

int Udp::UdpCmdCallbackRegister(string cmd, OnRpcCallbackFunc onRpcCallbackFunc)
{
	LOGI("UdpCmdCallbackRegister cmd: %s", cmd.c_str());
	onRpcCallbackFuncList[cmd] = onRpcCallbackFunc;
	return CODE_OK;
}

void Udp::UdpOnMessage(string message, struct sockaddr_in *si_other, int slen)
{
	LOGD("UdpOnMessage message: %s", message.c_str());
	Json::Value respValue;
	Json::Value payloadJson;
	if (payloadJson.parse(message) && payloadJson.isObject())
	{
		if (payloadJson.isMember("CMD") && payloadJson["CMD"].isString())
		{
			string cmd = payloadJson["CMD"].asString();
			if (onRpcCallbackFuncList.find(cmd) != onRpcCallbackFuncList.end())
			{
				OnRpcCallbackFunc onRpcCallbackFunc = onRpcCallbackFuncList[cmd];
				int rs = onRpcCallbackFunc(payloadJson, respValue);
				if (rs == CODE_OK)
				{
					LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
					send(respValue.toString(), si_other, slen);
				}
				else if (rs == CODE_DATA_ARRAY)
				{
					LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
					if (respValue.isArray())
					{
						for (auto &respV : respValue)
						{
							send(respV.toString(), si_other, slen);
							usleep(10000);
						}
					}
				}
				else if (rs == CODE_NOT_RESPONSE)
				{
					LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
				}
				else if (rs == CODE_EXIT)
				{
					LOGD("Call %s OK, rs: %d", cmd.c_str(), rs);
					send(respValue.toString(), si_other, slen);
					exit(1);
				}
				else
				{
					LOGW("Call %s ERR rs: %d", cmd.c_str(), rs);
				}
			}
			else
			{
				LOGW("Method %s not registed", cmd.c_str());
			}
		}
		else
		{
			LOGW("UdpOnMessage message: %s", message.c_str());
		}
	}
#ifdef CONFIG_USE_OLD_APP
	else
	{
		string messageBase64;
		string decode = macaron::Base64::Decode(message, messageBase64);
		if (decode == "")
		{
			if (payloadJson.parse(messageBase64) && payloadJson.isObject())
			{
				if (payloadJson.isMember("SSID") && payloadJson["SSID"].isString() &&
					payloadJson.isMember("PASSWORD") && payloadJson["PASSWORD"].isString() &&
					payloadJson.isMember("ENCRYPTION") && payloadJson["ENCRYPTION"].isString())
				{
					string ssid = payloadJson["SSID"].asString();
					string password = payloadJson["PASSWORD"].asString();
					string encryption = payloadJson["ENCRYPTION"].asString();
					LOGD("ssid: %s, password: %s, encryption: %s", ssid.c_str(), password.c_str(), encryption.c_str());
					if (Wifi::ConnectToWifi(ssid, password, encryption) == 0)
					{
						LOGI("ip: %s", Wifi::GetIP().c_str());
					}
					else
					{
						LOGW("Connect wifi err");
					}
				}
				else
				{
					LOGW("UdpOnMessage message: %s", messageBase64.c_str());
				}
			}
		}
		else
		{
			LOGW("Base64 decode err: %s", decode.c_str());
		}
	}
#endif
}

int Udp::send(string message, struct sockaddr_in *si_other, int slen)
{
	LOGI("Send message to IP: %s:%i, len: %d",
		 inet_ntoa(si_other->sin_addr), ntohs(si_other->sin_port), slen);
	LOGD("send message: %s", message.c_str());
	// now reply the client with the same data
	if (sendto(fd, message.c_str(), message.length(), 0, (struct sockaddr *)si_other, slen) == -1)
	{
		LOGW("UDP sendto error");
		return CODE_ERROR;
	}
	return CODE_OK;
}
