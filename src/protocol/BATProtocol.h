#ifdef CONFIG_ENABLE_MCU

#pragma once

#include <stdint.h>
#include <vector>
#include <Uart.h>

#define BAT_MAXLENGTH_MESSAMAXGE 58

class BATProtocol : public Uart
{
private:
	typedef struct
	{
		uint16_t header;
		uint8_t length;
		uint8_t type;
		uint16_t opcode;
		uint8_t data[BAT_MAXLENGTH_MESSAMAXGE];
	} message_st;

	vector<message_st *> messageRespList;

	int SendMessage(uint16_t opcode_req, uint8_t len_req, uint8_t *data_req,
									uint16_t opcode_resp, uint8_t *len_resp, uint8_t *data_resp, int timeout = 1000);
	void OnMessage(unsigned char *data, int len);

public:
	BATProtocol(char *uartPort, int uartBaudrate);
	virtual ~BATProtocol();

	int ControlRelay(uint8_t relay, uint8_t value, uint8_t &relayState);
	int ControlAllRelay(uint8_t allRelay, uint8_t &relayState);
	int RequestRelayState(uint8_t &relayState);
	int SetDimming(uint8_t id, uint8_t dimming);
};

extern BATProtocol *batProtocol;

#endif
