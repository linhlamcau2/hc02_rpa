#pragma once

#include <string>
#include <stdint.h>
#include <vector>
#include "Uart.h"
#include <atomic>
#include <functional>
#include <mutex>

using namespace std;

#define MESSAGE_MAXLENGTH 64
typedef struct __attribute__((packed))
{
	uint16_t header;
	uint16_t length;
	uint16_t opcode;
	uint8_t data[MESSAGE_MAXLENGTH];
} rl_rsp_st;

class RelayProtocol : public Uart
{
	int OnMessage(unsigned char *data, int len);

public:
	RelayProtocol(char *uartPort, int uartBaudrate);
	virtual ~RelayProtocol();
	void init();
};

extern RelayProtocol *relayProtocol;
