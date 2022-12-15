#ifdef CONFIG_ENABLE_MCU

#include "BATProtocol.h"
#include <stdlib.h>
#include <Log.h>
#include <Util.h>
#include <string.h>
#include <algorithm>

#define BAT_HEADER_MESSAGE 0xAA55

#define OPCODE_CONTROL_RELAY 0x0100
#define OPCODE_CONTROL_ALL_RELAY 0x0200
#define OPCODE_REQUEST_RELAY_STT 0x0300
#define OPCODE_RESPONSE_RELAY_STT 0x0400
#define OPCODE_REQUEST_DIMMING 0x0500
#define OPCODE_RESPONSE_DIMMING 0x0600

BATProtocol *batProtocol = NULL;

BATProtocol::BATProtocol(char *uartPort, int uartBaudrate) : Uart(uartPort, 100000)
{
	if (Open(uartBaudrate) < 0)
	{
		LOGE("Open uart error")
		exit(1);
	}
}

BATProtocol::~BATProtocol()
{
}

void BATProtocol::OnMessage(unsigned char *data, int len)
{
	LOGD("OnMessage len: %d", len);
	uint8_t *d = data;
	int l = len;
	message_st *message = NULL;
	while (l >= 7)
	{
		message = (message_st *)d;
		if (message->length <= 4 + BAT_MAXLENGTH_MESSAMAXGE)
		{
			uint8_t crc = Util::CalCrc(message->length, (uint8_t *)&message->length);
			if (crc == message->data[message->length - 4])
			{
				LOGD("RX opcode: 0x%02X", message->opcode);
				for (auto &messageResp : messageRespList)
				{
					if (messageResp->header == message->header && messageResp->opcode == message->opcode)
					{
						memcpy(messageResp, message, sizeof(message_st));
					}
				}
			}
			else
			{
				LOGW("Crc not match crc: 0x%02X, message->data[message->length - 4]: 0x%02X", crc, message->data[message->length - 4]);
			}
		}
		else
		{
			LOGW("message len error");
			return;
		}
		l -= message->length + 3;
		d += message->length + 3;
	}
}

int BATProtocol::SendMessage(uint16_t opcode_req, uint8_t len_req, uint8_t *data_req,
														 uint16_t opcode_resp, uint8_t *len_resp, uint8_t *data_resp, int timeout)
{
	int rs = 0;
	message_st message_resp = {
			.header = BAT_HEADER_MESSAGE,
			.length = 0,
			.type = 3,
			.opcode = opcode_resp};
	if (opcode_resp)
	{
		messageRespList.push_back(&message_resp);
	}

	message_st message_req = {
			.header = BAT_HEADER_MESSAGE,
			.length = (uint8_t)(len_req + 4),
			.type = 3,
			.opcode = opcode_req};
	for (int i = 0; i < len_req; i++)
	{
		message_req.data[i] = data_req[i];
	}
	message_req.data[len_req] = Util::CalCrc(len_req + 4, (uint8_t *)&message_req.length);

	Write((uint8_t *)&message_req, len_req + 7);

	if (opcode_resp)
	{
		while (message_resp.length == 0 && timeout)
		{
			usleep(1000);
			--timeout;
		}
		if (message_resp.length > 0)
		{
			if (len_resp)
			{
				*len_resp = message_resp.length - 4;
				if (data_resp)
					memcpy(data_resp, message_resp.data, *len_resp);
			}
		}
		else
			rs = 1;
		messageRespList.erase(remove(messageRespList.begin(), messageRespList.end(), &message_resp), messageRespList.end());
	}
	return rs;
}

int BATProtocol::ControlRelay(uint8_t relay, uint8_t value, uint8_t &relayState)
{
	LOGD("BAT ControlRelay %d value %d", relay, value);
	uint8_t len = 0;
	uint8_t data[64];
	struct
	{
		uint8_t relay;
		uint8_t value;
	} message_control_relay = {
			.relay = relay,
			.value = value};
	int rs = SendMessage(OPCODE_CONTROL_RELAY, sizeof(message_control_relay), (uint8_t *)&message_control_relay, OPCODE_RESPONSE_RELAY_STT, &len, data);
	if (!rs)
	{
		if (len == 1) {
			LOGD("Relay state: 0x%02X", data[0]);
			relayState = data[0];
			return 0;
		}
	}
	return 1;
}

int BATProtocol::ControlAllRelay(uint8_t allRelay, uint8_t &relayState)
{
	LOGD("BAT ControlAllRelay 0x%02X", allRelay);
	uint8_t len = 0;
	uint8_t data[64];
	struct
	{
		uint8_t all;
	} message_control_all_relay = {
			.all = allRelay};
	int rs = SendMessage(OPCODE_CONTROL_ALL_RELAY, sizeof(message_control_all_relay), (uint8_t *)&message_control_all_relay, OPCODE_RESPONSE_RELAY_STT, &len, data);
	if (!rs)
	{
		if (len == 1) {
			LOGD("Relay state: 0x%02X", data[0]);
			relayState = data[0];
			return 0;
		}
	}
	return 1;
}

int BATProtocol::RequestRelayState(uint8_t &relayState)
{
	LOGD("BAT RequestRelayState");
	uint8_t len = 0;
	uint8_t data[64];
	int rs = SendMessage(OPCODE_REQUEST_RELAY_STT, 0, NULL, OPCODE_RESPONSE_RELAY_STT, &len, data);
	if (!rs)
	{
		if (len == 1) {
			LOGD("Relay state: 0x%02X", data[0]);
			relayState = data[0];
			return 0;
		}
	}
	return 1;
}

int BATProtocol::SetDimming(uint8_t id, uint8_t dimming)
{
	LOGD("BAT SetDimming %d dimming %d", id, dimming);
	uint8_t len = 0;
	uint8_t data[64];
	typedef struct
	{
		uint8_t id;
		uint8_t dimming;
	} message_get_dimming_st;

	struct
	{
		uint8_t id;
		uint8_t dimming;
	} message_set_dimming = {
			.id = id,
			.dimming = dimming};

	SendMessage(OPCODE_REQUEST_DIMMING, sizeof(message_set_dimming), (uint8_t *)&message_set_dimming, OPCODE_RESPONSE_DIMMING, &len, data);

	if (len == sizeof(message_get_dimming_st))
	{
		message_get_dimming_st *message_get_dimming = (message_get_dimming_st *)data;
		LOGI("Dimming resp id %d dimming: %d", message_get_dimming->id, message_get_dimming->dimming);
		return 0;
	}
	return 1;
}

#endif
