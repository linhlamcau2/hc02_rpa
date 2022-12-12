#ifdef CONFIG_ENABLE_LORA

#include "LoraProtocol.h"
#include <stdlib.h>
#include <thread>
#include <functional>
#include <byteswap.h>
#include <Log.h>
#include <Util.h>
#include <string.h>
#include <algorithm>
#include <Db.h>
#include "DeviceLora.h"

typedef struct
{
	uint32_t addr;
	uint8_t data[50];
} data_sensor_st;

LoraProtocol *loraProtocol = NULL;

LoraProtocol::LoraProtocol(char *uartPort, int uartBaudrate) : Uart(uartPort, 100000)
{
	isDeviceScanning = false;
	if (Open(uartBaudrate) < 0)
	{
		LOGE("Open uart error")
		exit(1);
	}
}

LoraProtocol::~LoraProtocol()
{
}

void LoraProtocol::CheckOpcodeException(message_st *message)
{
	if (message->header == LORA_HEADER_MESSAGE)
	{
		switch (message->opcode)
		{
		case OPCODE_START_SCAN_RESP:
		{
			if (message->length <= 4)
				return;
			scan_resp_st *scan_resp = (scan_resp_st *)message->data;
			uint32_t type = scan_resp->type;
			string mac = Util::ConvertU32ToHexString(scan_resp->mac, sizeof(scan_resp->mac));
			LOGI("Scan new device mac: 0x%s, addr: 0x%04X, type: %d", mac.c_str(), scan_resp->addr, scan_resp->type);
			Device *device = gateway->getDevice(mac);
			// if (device)
			// {
			// 	device->SetAddr(scan_resp->addr);
			// 	database->DeviceUpdate(device);
			// }
			// else if (type == LORA_SENSOR_LIGHT)
			// {
			// 	device = gateway->AddNewDevice("Lora_" + mac, "Cảm biến ánh sáng Lora", mac, scan_resp->addr, type, true, true);
			// 	gateway->AddDeviceToScanList(device);
			// }
			// else if (type == LORA_SENSOR_SOIL_MOISTURE)
			// {
			// 	device = gateway->AddNewDevice("Lora_" + mac, "Cảm biến độ ẩm đất Lora", mac, scan_resp->addr, type, true, true);
			// 	gateway->AddDeviceToScanList(device);
			// }
			// else if (type == LORA_SENSOR_HUM_TEMP)
			// {
			// 	device = gateway->AddNewDevice("Lora_" + mac, "Cảm biến nhiệt ẩm Lora", mac, scan_resp->addr, type, true, true);
			// 	gateway->AddDeviceToScanList(device);
			// }
			// else if (type == LORA_SENSOR_CO2)
			// {
			// 	device = gateway->AddNewDevice("Lora_" + mac, "Cảm biến CO2 Lora", mac, scan_resp->addr, type, true, true);
			// 	gateway->AddDeviceToScanList(device);
			// }
			// else if (type == LORA_SENSOR_EC)
			// {
			// 	device = gateway->AddNewDevice("Lora_" + mac, "Cảm biến EC Lora", mac, scan_resp->addr, type, true, true);
			// 	gateway->AddDeviceToScanList(device);
			// }
			// else
			// {
			// 	LOGW("Lora device type 0x%08X not add", type);
			// }
			// scanDeviceList.push_back(*scan_resp);
			break;
		}
		case OPCODE_LIGHT_SENSOR:
		case OPCODE_SOIL_MOIS_SENSOR:
		case OPCODE_TEMP_HUM_SENSOR:
		case OPCODE_CO2_SENSOR:
		case OPCODE_SOIL_TEMP_MOIS_SENSOR:
		{
			data_sensor_st *data_sensor = (data_sensor_st *)message->data;
			LOGI("Data sensor addr: 0x%04X", data_sensor->addr);
			DeviceLora *deviceLora = gateway->getDeviceLoraFromAddr(data_sensor->addr);
			if (deviceLora) // && device->GetType() == bswap_16(message->opcode))
			{
				LOGI("Have device mac %s type: %d", deviceLora->GetMac().c_str(), deviceLora->GetType());
				deviceLora->InputData(data_sensor->data, -1);
				ResponseDevice(data_sensor->addr);
			}
			else
			{
				LOGW("Not found device addr: 0x%02X", data_sensor->addr);
			}
			break;
		}
		default:
			LOGW("Opcode not support");
			break;
		}
	}
}

void LoraProtocol::OnMessage(unsigned char *data, int len)
{
	LOGD("OnMessage len: %d", len);
	uint8_t *d = data;
	int l = len;
	message_st *message = NULL;
	while (l >= 7)
	{
		message = (message_st *)d;
		if (message->length <= 4 + LORA_MAXLENGTH_MESSAMAXGE)
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
				CheckOpcodeException(message);
			}
			else
			{
				LOGW("Crc not match crc: 0x%02X, message->data[message->length - 4]: 0x%02X", crc, message->data[message->length - 4]);
			}
		}
		else
		{
			LOGW("message len error");
		}
		l -= message->length + 3;
		d += message->length + 3;
	}
}

int LoraProtocol::SendMessage(uint16_t opcode_req, uint8_t len_req, uint8_t *data_req,
															uint16_t opcode_resp, uint8_t *len_resp, uint8_t *data_resp, int timeout)
{
	int rs = 0;
	message_st message_resp = {
			.header = LORA_HEADER_MESSAGE,
			.length = 0,
			.type = 2,
			.opcode = opcode_resp};
	if (opcode_resp)
	{
		messageRespList.push_back(&message_resp);
	}

	message_st message_req = {
			.header = LORA_HEADER_MESSAGE,
			.length = (uint8_t)(len_req + 4),
			.type = 2,
			.opcode = opcode_req};
	for (int i = 0; i < len_req; i++)
	{
		message_req.data[i] = data_req[i];
	}
	message_req.data[len_req] = Util::CalCrc(len_req + 4, (uint8_t *)&message_req.length);
	message_req.data[len_req + 1] = 0x09;
	message_req.data[len_req + 2] = 0x0A;

	Write((uint8_t *)&message_req, len_req + 9);

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

// vector<scan_resp_st> *LoraProtocol::StartScan(int timeout)
int LoraProtocol::StartScan()
{
	return SendMessage(OPCODE_START_SCAN, 0, NULL, 0, NULL, NULL);
}

// vector<scan_resp_st> *LoraProtocol::StartScan(int timeout)
int LoraProtocol::StartScan(vector<scan_resp_st> *&scanDeviceList, int timeout)
{
	if (isDeviceScanning)
		return 1;
	isDeviceScanning = true;
	this->scanDeviceList.clear();
	SendMessage(OPCODE_START_SCAN, 0, NULL, 0, NULL, NULL);
	time_t expiredTime = time(NULL) + timeout;
	while (time(NULL) < expiredTime)
	{
		usleep(100000);
	}
	isDeviceScanning = false;
#if 0
uint8_t bytes[] = {0x55, 0xAA, 0x0D, 0x02, 0x00, 0x01, 0xA4, 0xC5, 0x06, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x69};
OnMessage(bytes, sizeof(bytes));
#endif
#if 0 // test
	scan_resp_st scan_resp = {
			.mac = 1,
			.addr = 2,
			.type = bswap_16(OPCODE_LIGHT_SENSOR)};
	this->scanDeviceList.push_back(scan_resp);
	scan_resp_st scan_resp2 = {
			.mac = 3,
			.addr = 4,
			.type = bswap_16(OPCODE_SOIL_MOIS_SENSOR)};
	this->scanDeviceList.push_back(scan_resp2);
#endif
	StopScan();
	scanDeviceList = &this->scanDeviceList;
	return 0;
}

int LoraProtocol::StopScan()
{
	return 0;
}

int LoraProtocol::ResponseDevice(uint32_t addr)
{
	typedef struct
	{
		uint16_t header;
		uint8_t length;
		uint8_t type;
		uint16_t opcode;
		uint32_t addr;
		uint8_t data[LORA_MAXLENGTH_MESSAMAXGE];
	} response_message_st;
	response_message_st message_req = {
			.header = LORA_HEADER_MESSAGE,
			.length = 8,
			.type = 2,
			.opcode = OPCODE_RESPONSE_DEVICE,
			.addr = addr};
	message_req.data[0] = Util::CalCrc(8, (uint8_t *)&message_req.length);
	message_req.data[1] = 0x09;
	message_req.data[2] = 0x0A;

	Write((uint8_t *)&message_req, 17);
	return 0;
}

int LoraProtocol::DelDevice(string mac)
{
	return 0;
}

int LoraProtocol::SetTimeResponse(string mac)
{
	return 0;
}

#endif
