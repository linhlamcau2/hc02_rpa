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

class QrProtocol : public Uart
{
	int OnMessage(unsigned char *data, int len);

public:
	string mac;
	uint16_t addr;
	string mac_k9b;
	uint32_t mac_k9b_int;
	bool isMac_k9b;
	bool startTest;
#ifdef ESP_PLATFORM
	QrProtocol(uart_port_t num, int txPin, int rxPin, int baudrate);
#else
	QrProtocol(char *uartPort, int uartBaudrate);
#endif
	virtual ~QrProtocol();
	void init();
};

extern QrProtocol *qrProtocol;
