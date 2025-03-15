#pragma once

#include <string>
#include <stdint.h>
#include <vector>
#include "Uart.h"
#include <atomic>
#include <functional>
#include <mutex>

using namespace std;

class QrProtocol : public Uart
{
	int OnMessage(unsigned char *data, int len);

public:
	string mac;
	uint16_t addr;
	bool startTest;

	QrProtocol(char *uartPort, int uartBaudrate);
	virtual ~QrProtocol();
	void init();
};

extern QrProtocol *qrProtocol;
