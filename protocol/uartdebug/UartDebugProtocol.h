#pragma once

#include <string>
#include <stdint.h>
#include <vector>
#include "Uart.h"
#include <atomic>
#include <functional>
#include <mutex>
#include <atomic>
using namespace std;

class UartDebugProtocol : public Uart
{
	int OnMessage(unsigned char *data, int len);
public:
	UartDebugProtocol(char *uartPort, int uartBaudrate);
	virtual ~UartDebugProtocol();
	void init();
	int SetValueButton(uint8_t button);
};

extern UartDebugProtocol *uartDebugProtocol;
