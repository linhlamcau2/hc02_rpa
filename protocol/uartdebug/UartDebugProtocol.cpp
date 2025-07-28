#include "UartDebugProtocol.h"
#include <stdlib.h>
#include <thread>
#include <functional>
#include <byteswap.h>
#include "Log.h"
#include "Util.h"
#include <string.h>
#include <algorithm>
#include "Config.h"

#define MESSAGE_MAXLENGTH 64

UartDebugProtocol *uartDebugProtocol = NULL;

typedef struct __attribute__((packed))
{
	uint16_t header;
	uint16_t length;
	uint16_t opcode;
	uint8_t data[MESSAGE_MAXLENGTH];
} bt_rsp_st;

UartDebugProtocol::UartDebugProtocol(char *uartPort, int baudrate) : Uart(uartPort, baudrate, 100000)
{
}

UartDebugProtocol::~UartDebugProtocol()
{
}

void UartDebugProtocol::init()
{
	Uart::init();
	SLEEP_MS(100); // wait for thread start
}

uint8_t calc_crc(const char *buf) {
    uint16_t sum = 0;
    for (int i = 2; i <= 6; i++) {
        sum += (uint8_t)buf[i];
    }
    return (uint8_t)(sum & 0xFF);
}

int UartDebugProtocol::SetValueButton(uint8_t button)
{
    char buf[8];
    buf[0] = 0x55;    // header
    buf[1] = (char)0xAA;
    buf[2] = 0x00;    // lengt
    buf[3] = 0x01;
    buf[4] = 0x03;    // opcode
    buf[5] = 0x00;
    buf[6] = (char)button; // BT
    buf[7] = calc_crc(buf);  
    return Write(buf, 8);
}

int UartDebugProtocol::OnMessage(unsigned char *data, int len)
{
	uint8_t *d = data;
	int l = len;
	bt_rsp_st *message_rsp = NULL;
	while (l >= 7)
	{
		message_rsp = (bt_rsp_st *)d;
		if (message_rsp->length >= 3 && message_rsp->length <= 68)
		{
			if (message_rsp->header == 0x55aa)
			{
				LOGD("UartDebugProtocol::OnMessage: header=0x%04x, length=%d, opcode=0x%04x", message_rsp->header, message_rsp->length, message_rsp->opcode);
				if (message_rsp->length + 5 > l)
				{
					LOGE("UartDebugProtocol::OnMessage: Invalid length %d, expected at least %d bytes", message_rsp->length, message_rsp->length + 5);
					return -1;
				}
				if (message_rsp->opcode == 0x03) // Button response
				{
					if (message_rsp->data[0] == 0x01) // Button pressed
					{
						LOGD("Button %d pressed", message_rsp->data[1]);
					}
					else if (message_rsp->data[0] == 0x00) // Button released
					{
						LOGD("Button %d released", message_rsp->data[1]);
					}
				}
				else if (message_rsp->opcode == 0x04) // Other opcode
				{
					LOGD("Received opcode 0x04 with data: ");
					for (int i = 0; i < message_rsp->length - 3; i++)
					{
						LOGD("0x%02x ", message_rsp->data[i]);
					}
					LOGD("");
				}
				// Process the message as needed
				// For example, you can call a callback function or store the data in a member variable
				// Here we just log the received data
				LOGD("Received data: ");
				for (int i = 0; i < message_rsp->length - 3; i++)
				{
					LOGD("0x%02x ", message_rsp->data[i]);
				}
				LOGD("");		
				uint16_t packageLen = message_rsp->length + 5;
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
