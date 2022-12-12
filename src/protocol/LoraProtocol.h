#ifdef CONFIG_ENABLE_LORA

#pragma once

#include <stdint.h>
#include <vector>
#include <Uart.h>

#define LORA_HEADER_MESSAGE 0xAA55

#define OPCODE_START_SCAN 0x0100
#define OPCODE_START_SCAN_RESP 0x0100
#define OPCODE_RESPONSE_DEVICE 0x0400
#define OPCODE_LIGHT_SENSOR 0x0400
#define OPCODE_SOIL_MOIS_SENSOR 0x0500
#define OPCODE_TEMP_HUM_SENSOR 0x0600
#define OPCODE_CO2_SENSOR 0x0700
#define OPCODE_SOIL_TEMP_MOIS_SENSOR 0x0800

#define LORA_MAXLENGTH_MESSAMAXGE 58

using namespace std;

typedef struct
{
	uint8_t mac[4];
	uint32_t addr;
	uint8_t type;
} scan_resp_st;

class LoraProtocol : public Uart
{
private:
	typedef struct
	{
		uint16_t header;
		uint8_t length;
		uint8_t type;
		uint16_t opcode;
		uint8_t data[LORA_MAXLENGTH_MESSAMAXGE];
	} message_st;

	// static message_st message_resp;
	volatile bool isDeviceScanning;
	vector<message_st *> messageRespList;
	vector<scan_resp_st> scanDeviceList;
	void CheckOpcodeException(message_st *message);
	int SendMessage(uint16_t opcode_req, uint8_t len_req, uint8_t *data_req,
									uint16_t opcode_resp, uint8_t *len_resp, uint8_t *data_resp, int timeout = 1000);
	void OnMessage(unsigned char *data, int len);

public:
	LoraProtocol(char *uartPort, int uartBaudrate);
	virtual ~LoraProtocol();

	int StartScan();
	int StartScan(vector<scan_resp_st> *&scanDeviceList, int timeout);
	int StopScan();
	int ResponseDevice(uint32_t addr);
	int DelDevice(string mac);
	int SetTimeResponse(string mac);
};

extern LoraProtocol *loraProtocol;

#endif
