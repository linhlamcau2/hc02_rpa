#include "RelayProtocol.h"
#include "BleProtocol.h"
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
				LOGD("header: %04X", message_rsp->header);
				LOGD("length: %d", message_rsp->length);
				LOGD("opcode: %04X", message_rsp->opcode);
				LOGD("Data:");
				for (int i = 0; i < (message_rsp->length); i++)
				{
					printf("%d ", message_rsp->data[i]);
				}
				printf("\n");
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