#include "RelayProtocol.h"
#include <stdlib.h>
#include <thread>
#include <functional>
#include <byteswap.h>
#include "Log.h"
#include "Util.h"
#include <string.h>
#include <algorithm>
#include "BleOpCode.h"

RelayProtocol *relayProtocol = NULL;

RelayProtocol::RelayProtocol(char *uartPort, int baudrate) : Uart(uartPort, baudrate, 100000)
{
	this->rl1 = 0;
	this->rl2 = 0;
	this->rl3 = 0;
	this->rl4 = 0;
}

RelayProtocol::~RelayProtocol()
{
}

void RelayProtocol::init()
{
	Uart::init();
	SLEEP_MS(100); // wait for thread start
}

int RelayProtocol::OnMessage(unsigned char *data, int len)
{
	uint8_t *d = data;
	int l = len;
	rl_rsp_st *message_rsp = NULL;
	while (l >= 7)
	{
		message_rsp = (rl_rsp_st *)d;
		if (message_rsp->length >= 3 && message_rsp->length <= 68)
		{
			if (message_rsp->header == 0x55aa)
			{
				uint16_t packageLen = message_rsp->length + 5;
				this->rl1 = !message_rsp->data[0];
				this->rl2 = !message_rsp->data[1];
				this->rl3 = !message_rsp->data[2];
				this->rl4 = !message_rsp->data[3];
				// LOGE("rl1: %d, rl2: %d, rl3: %d, rl4: %d", rl1, rl2, rl3, rl4);
				l -= packageLen;
				d += packageLen;
			}
			else
			{
				d++;
				l--;
			}
		}
		else
		{
			d++;
			l--;
		}
		usleep(300);
	}
	return l;
}