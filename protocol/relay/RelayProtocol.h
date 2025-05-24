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
	int rl1;
	int rl2;
	int rl3;
	int rl4;

#ifdef ESP_PLATFORM
	RelayProtocol(uart_port_t num, int txPin, int rxPin, int baudrate);
#else
	RelayProtocol(char *uartPort, int uartBaudrate);
#endif
	virtual ~RelayProtocol();
	int GetStatusRelay(int index);
	void init();
};

extern RelayProtocol *relayProtocol;
