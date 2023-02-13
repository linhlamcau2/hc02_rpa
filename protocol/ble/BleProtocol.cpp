#include "BleProtocol.h"
#include <stdlib.h>
#include <thread>
#include <functional>
#include <byteswap.h>
#include <Log.h>
#include <Util.h>
#include <string.h>
#include <algorithm>
#include <Db.h>
#include "BleOpCode.h"
#include "DeviceBle.h"
#include "AES.h"

BleProtocol *bleProtocol = NULL;

static uint8_t keyAes[] = {0x44, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6c, 0x40, 0x32, 0x38, 0x31, 0x31, 0x32, 0x38, 0x30, 0x34};
static uint8_t plaintext[] = {0x24, 0x02, 0x28, 0x04, 0x28, 0x11, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// static uint8_t appKey[] = {0x60, 0x96, 0x47, 0x71, 0x73, 0x4f, 0xbd, 0x76, 0xe3, 0xb4, 0x05, 0x19, 0xd1, 0xd9, 0x4a, 0x48};

#ifdef ESP_PLATFORM
BleProtocol::BleProtocol(int num, int txPin, int rxPin, int baudrate) : Uart(num, txPin, rxPin, baudrate)
{
#else
BleProtocol::BleProtocol(char *uartPort, int uartBaudrate) : Uart(uartPort, 100000)
{
	if (Open(uartBaudrate) < 0)
	{
		LOGE("Open uart error")
		exit(1);
	}
#endif
	nextAddr = 0;
	isAdding = false;
}

BleProtocol::~BleProtocol()
{
	isAdding = false;
}

void BleProtocol::init()
{
	Uart::init();
	usleep(100000); // wait uart rx thread start
	addDeviceFunc = bind(&BleProtocol::AddDevice, this, placeholders::_1);
	while (GetNetKey())
	{
		sleep(5);
	}
	GetAppKey();
	database->GatewayRead();
}

void BleProtocol::CheckOpcodeException(message_rsp_st *message_rsp)
{
	// LOGD("CheckOpcodeException");
	switch (message_rsp->opcode)
	{
	case HCI_GATEWAY_CMD_UPDATE_MAC:
		if (isAdding)
		{
			isAdding = false;
			memcpy(&scanDeviceMessage, message_rsp->data, sizeof(scan_device_message_t));
			thread addDeviceThread(addDeviceFunc, &scanDeviceMessage);
			addDeviceThread.detach();
		}
		break;

	case HCI_GATEWAY_RSP_OP_CODE:
	{
		typedef struct
		{
			uint16_t dev_addr;
			uint16_t gw_addr;
			uint8_t data[100];
		} data_message_t;
		data_message_t *data_message = (data_message_t *)message_rsp->data;
		LOGD("Device addr 0x%04X", data_message->dev_addr);
		DeviceBle *deviceBle = gateway->getDeviceBleFromAddr(data_message->dev_addr);
		if (deviceBle)
		{
			LOGD("Have device mac 0x%s type: 0x%08X", deviceBle->GetMac().c_str(), deviceBle->GetType());
			deviceBle->DeviceInputData(data_message->data, message_rsp->len - 6, data_message->dev_addr);
		}
		else
		{
			LOGW("Not found device addr: 0x%04X", data_message->dev_addr);
		}
		break;
	}
	case HCI_GATEWAY_CMD_SEND_NODE_INFO:
	{
		for (int i = 0; i < 16; i++)
		{
			deviceKey[i] = message_rsp->data[i + 4];
		}
		break;
	}

	default:
		break;
	}
}

int BleProtocol::OnMessage(unsigned char *data, int len)
{
	LOGD("OnMessage len: %d", len);
	uint8_t *d = data;
	int l = len;
	message_rsp_st *message_rsp = NULL;
	message_rsp_st *old_message_rsp = NULL;
	bool is_dupplicate = false;
	bool match;
	Util::LedBle(false);
	Util::LedServiceLock();
	while (l >= 4)
	{
		message_rsp = (message_rsp_st *)d;
		is_dupplicate = false;
		if (old_message_rsp && message_rsp->len == old_message_rsp->len)
		{
			is_dupplicate = true;
			for (int i = 0; i < message_rsp->len; i++)
			{
				if (message_rsp->data[i] != old_message_rsp->data[i])
				{
					is_dupplicate = false;
					break;
				}
			}
		}
		if (!is_dupplicate)
		{
			if (message_rsp->len >= 2 && message_rsp->len <= l - 2)
			{
				LOGD("onMessage opcode: 0x%02X, len: %d", message_rsp->opcode, message_rsp->len);
				for (auto &messageResp : messageRespList)
				{
					if (message_rsp->opcode == messageResp->opcode)
					{
						match = true;
						if (messageResp->compare_data)
						{
							for (int i = 0; i < messageResp->compare_len; i++)
							{
								if (message_rsp->data[messageResp->compare_position + i] != messageResp->compare_data[i])
									match = false;
							}
						}
						if (match)
						{
							messageResp->status = true;
							if (messageResp->len)
							{
								*(messageResp->len) = message_rsp->len - 2;
								if (messageResp->data)
									memcpy(messageResp->data, message_rsp->data, *messageResp->len);
							}
						}
					}
				}
				CheckOpcodeException(message_rsp);
			}
			else
			{
				break;
			}
		}
		else
		{
			// LOGW("dupplicate");
		}
		old_message_rsp = message_rsp;
		l -= message_rsp->len + 2;
		d += message_rsp->len + 2;
	}
	Util::LedBle(true);
	Util::LedServiceUnlock();
	return l;
}

int BleProtocol::SendMessage(uint16_t opReq, uint8_t *dataReq, int lenReq, uint8_t opRsp, uint8_t *dataRsp, int *lenRsp, uint32_t timeout, uint8_t *compare_data, int compare_position, int compare_len)
{
	int rs = 0;
	message_rsp_list_st message_rsp_list = {
			.status = false,
			.opcode = opRsp,
			.len = lenRsp,
			.data = dataRsp,
			.compare_data = compare_data,
			.compare_position = compare_position,
			.compare_len = compare_len};
	if (opRsp)
	{
		messageRespList.push_back(&message_rsp_list);
	}

	message_req_st message_req = {
			.opcode = opReq};
	for (int i = 0; i < lenReq; i++)
	{
		message_req.data[i] = dataReq[i];
	}

	Write((uint8_t *)&message_req, lenReq + 2);

	if (opRsp)
	{
		while (!message_rsp_list.status && timeout)
		{
			usleep(1000);
			--timeout;
		}
		if (message_rsp_list.status)
		{
		}
		else
			rs = -1;
		messageRespList.erase(remove(messageRespList.begin(), messageRespList.end(), &message_rsp_list), messageRespList.end());
	}
	else
	{
		usleep(1000 * timeout);
	}
	return rs;
	// return Write(dataReq, lenReq);
}

int BleProtocol::GetAppKey()
{
	string appkeyStr = gateway->getBleAppKey();
	if (appkeyStr.compare("") == 0)
	{
		LOGD("Appkey null");
		srand((int)time(0));
		for (int i = 0; i < 16; i++)
		{
			appKey[i] = rand() % 256;
		}
		string appkey = arrayToString844412((uint8_t *)appKey);
		LOGD("New ble_appkey: %s", appkey.c_str());
		database->GatewayUpdateAppKey(gateway, appkey);
	}
	else
	{
		LOGD("Appkey: %s", appkeyStr.c_str());
		appkeyStr.erase(appkeyStr.begin() + 8, appkeyStr.begin() + 9);
		appkeyStr.erase(appkeyStr.begin() + 12, appkeyStr.begin() + 13);
		appkeyStr.erase(appkeyStr.begin() + 16, appkeyStr.begin() + 17);
		appkeyStr.erase(appkeyStr.begin() + 20, appkeyStr.begin() + 21);
		char *ak = new char[appkeyStr.length() + 1];
		strcpy(ak, appkeyStr.c_str());
		uint8_t temp[17] = {0};
		for (int i = 0; i < 16; i++)
		{
			sscanf((char *)ak + i * 2, "%2x", (unsigned int *)&temp[i]);
			appKey[i] = temp[i];
		}
	}
	return 0;
}

int BleProtocol::GetNetKey()
{
	LOGD("GetNetKey");
	uint8_t d = HCI_GATEWAY_CMD_GET_PRO_SELF_STS;
	uint8_t dataRsp[100];
	int lenRsp;
	int rs = SendMessage(SYSTEM_REQ, &d, 1, HCI_GATEWAY_CMD_PRO_STS_RSP, dataRsp, &lenRsp, 2000);
	if (rs == 0)
	{
		typedef struct
		{
			uint8_t rev1;
			uint8_t netKey[16];
			uint8_t rev2[3];
			uint8_t magic[4];
			uint8_t addr[2];
		} data_message_t;
		data_message_t *data_message = (data_message_t *)dataRsp;

		uint32_t ivIndex = (data_message->magic[0] << 24) | (data_message->magic[1] << 16) | (data_message->magic[2] << 8) | (data_message->magic[3]);
		if ((ivIndex == 0x11223344) || (ivIndex == 0))
		{
			for (int i = 0; i < 16; i++)
			{
				netKey[i] = data_message->netKey[i];
			}
			nextAddr = data_message->addr[0] | (data_message->addr[1] << 8);
			if (nextAddr == 0)
				nextAddr = 2;
			LOGW("nextAddr: 0x%04X - %d", nextAddr, nextAddr);
		}
		else
		{
			srand((int)time(0));
			for (int i = 0; i < 16; i++)
			{
				netKey[i] = rand() % 256;
				gwKey[i] = rand() % 256;
			}
			SetNetKey();
			SetGwKey();
			string netkeyStr = arrayToString844412((uint8_t *)netKey);
			LOGD("New ble_netkey: %s", netkeyStr.c_str());
			database->GatewayUpdateNetKey(gateway, netkeyStr);

			string devicekeyGwStr = arrayToString844412((uint8_t *)gwKey);
			LOGD("New ble_devicekeyGw: %s", devicekeyGwStr.c_str());
			database->GatewayUpdateDeviceKey(gateway, devicekeyGwStr);

			rs = 1;
		}
	}
	else
	{
		LOGE("Send GetNetKey error, rs: %d", rs);
		rs = 1;
	}
	return rs;
}

int BleProtocol::SetNetKey()
{
	LOGD("SetNetKey");
	typedef struct
	{
		uint8_t opcode;
		uint8_t netKey[16];
		uint8_t rev[3];
		uint8_t magic[4];
		uint8_t addr[2];
	} set_netkey_message_t;
	set_netkey_message_t set_netkey_message;
	memset(&set_netkey_message, 0x00, sizeof(set_netkey_message));
	set_netkey_message.opcode = HCI_GATEWAY_CMD_SET_PRO_PARA;
	for (int i = 0; i < 16; i++)
	{
		set_netkey_message.netKey[i] = netKey[i];
	}
	set_netkey_message.magic[0] = 0x11;
	set_netkey_message.magic[1] = 0x22;
	set_netkey_message.magic[2] = 0x33;
	set_netkey_message.magic[3] = 0x44;
	set_netkey_message.addr[0] = 0x01;
	set_netkey_message.addr[1] = 0x00;
	uint16_t adrGw = set_netkey_message.addr[0] | (set_netkey_message.addr[1] << 8);
	gateway->setBleUnicast(adrGw);
	database->GatewayUpdateUnicast(gateway, adrGw);
	return SendMessage(SYSTEM_REQ, (uint8_t *)&set_netkey_message, 26, HCI_GATEWAY_CMD_SEND_IVI, 0, 0, 1000);
}

int BleProtocol::SetGwKey()
{
	LOGD("SetGwKey");
	typedef struct
	{
		uint8_t opcode;
		uint8_t addr[2];
		uint8_t gwKey[16];
	} set_gwkey_message_t;
	set_gwkey_message_t set_gwkey_message;
	set_gwkey_message.opcode = 0x0D;
	set_gwkey_message.addr[0] = 0x01;
	set_gwkey_message.addr[1] = 0x00;
	for (int i = 0; i < 16; i++)
	{
		set_gwkey_message.gwKey[i] = gwKey[i];
	}
	return SendMessage(SYSTEM_REQ, (uint8_t *)&set_gwkey_message, 19, 0, 0, 0, 1000);
}

int BleProtocol::StartScan()
{
	LOGD("StartScan BLE");
	uint8_t d = HCI_GATEWAY_CMD_START;
	int rs = SendMessage(SYSTEM_REQ, &d, 1, 0, 0, 0, 5000);
	if (rs)
	{
		LOGE("Send start scan error, rs: %d", rs);
		return 1;
	}
	return 0;
}

int BleProtocol::StopScan()
{
	LOGD("StopScan");
	uint8_t d = HCI_GATEWAY_CMD_STOP;
	int rs = SendMessage(SYSTEM_REQ, &d, 1, 0, 0, 0, 500);
	if (rs)
	{
		LOGE("Send stop scan error, rs: %d", rs);
		return 1;
	}
	return 0;
}

int BleProtocol::ResetFactory()
{
	LOGD("ResetFactory");
	uint8_t d = HCI_GATEWAY_CMD_RESET;
	int rs = SendMessage(SYSTEM_REQ, &d, 1, 0, 0, 0, 5000);
	if (rs)
	{
		LOGE("Send reset factory error, rs: %d", rs);
		return 1;
	}
	while (GetNetKey())
	{
		sleep(5);
	}
	GetAppKey();
	database->GatewayRead();
	return 0;
}

string BleProtocol::uuidToStr(uuid_t *uuid)
{
	char buf[100];
	uint8_t *u8Uuid = (uint8_t *)uuid;
	sprintf(buf, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					u8Uuid[0], u8Uuid[1], u8Uuid[2], u8Uuid[3],
					u8Uuid[4], u8Uuid[5], u8Uuid[6], u8Uuid[7],
					u8Uuid[8], u8Uuid[9], u8Uuid[10], u8Uuid[11],
					u8Uuid[12], u8Uuid[13], u8Uuid[14], u8Uuid[15]);
	buf[36] = '\0';
	return string(buf);
}

string BleProtocol::arrayToString844412(uint8_t *array)
{
	char buf[100];
	sprintf(buf, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					array[0], array[1], array[2], array[3],
					array[4], array[5], array[6], array[7],
					array[8], array[9], array[10], array[11],
					array[12], array[13], array[14], array[15]);
	buf[36] = '\0';
	return string(buf);
}

static uint32_t convertDeviceType(uint32_t type)
{
	uint8_t *arr = (uint8_t *)&type;
	return (arr[0] + (arr[1] * 1000) + (arr[2] * 10000));
}

bool BleProtocol::AddDevice(scan_device_message_t *scan_device_message)
{
	LOGD("AddDevice");
	uint16_t version = 0;
	uint32_t deviceType = 0;
	uuid_t *uuid = (uuid_t *)scan_device_message->uuid;
	string mac = Util::ConvertU32ToHexString(scan_device_message->mac, sizeof(scan_device_message->mac));
	LOGI("Scan device mac 0x%s, rssi: %i", mac.c_str(), scan_device_message->rssi);
	if (isProvisioning && !SelectMac(scan_device_message->mac))
	{
		if (isProvisioning && !GetNetKey())
		{
			if (isProvisioning && !Provision(nextAddr))
			{
				if (isProvisioning && !BindingAll())
				{
					if (isProvisioning && !SetGwAddr(nextAddr, gateway->getBleUnicast()))
					{
						if (isProvisioning && !GetDeviceType(scan_device_message->mac, nextAddr, deviceType, version))
						{
							deviceType = convertDeviceType(deviceType);
							Device *device = gateway->AddNewDevice(uuidToStr(uuid), Device::ConvertDeviceTypeToName(deviceType), mac, arrayToString844412((uint8_t *)deviceKey), nextAddr, deviceType, version, true, true);
							if (device)
							{
								gateway->AddDeviceToScanList(device);
								isAdding = true;
								StartScan();
								return false;
							}
							else
							{
								ResetDev(nextAddr);
								isAdding = true;
								StartScan();
								return false;
							}
						}
						else
						{
							ResetDev(nextAddr);
							if (!isProvisioning)
							{
								isAdding = false;
								return false;
							}
							else
							{
								LOGW("GetDeviceType false");
								isAdding = true;
								StartScan();
								return false;
							}
						}
					}
					else
					{
						ResetDev(nextAddr);
						if (!isProvisioning)
						{
							isAdding = false;
							return false;
						}
						else
						{
							LOGW("SetGwAddr false");
							isAdding = true;
							StartScan();
							return false;
						}
					}
				}
				else
				{
					ResetDev(nextAddr);
					if (!isProvisioning)
					{
						isAdding = false;
						return false;
					}
					else
					{
						LOGW("BindingAll false");
						isAdding = true;
						StartScan();
						return false;
					}
				}
			}
			else
			{
				ResetDev(nextAddr);
				if (!isProvisioning)
				{
					isAdding = false;
					return false;
				}
				else
				{
					LOGW("Provision false");
					isAdding = true;
				}
			}
		}
		else
		{
			if (!isProvisioning)
			{
				isAdding = false;
				return false;
			}
			else
			{
				LOGW("Get NWK false");
				isAdding = true;
			}
		}
	}
	else
	{
		if (!isProvisioning)
		{
			isAdding = false;
			return false;
		}
		else
		{
			LOGW("Select Mac false");
			isAdding = true;
		}
	}
	isAdding = false;
	return false;
}

int BleProtocol::SelectMac(uint8_t *mac)
{
	LOGD("SelectMac");
	uint8_t data[7];
	data[0] = HCI_GATEWAY_CMD_SET_ADV_FILTER;
	for (int i = 0; i < 6; i++)
	{
		data[i + 1] = mac[i];
	}
	return SendMessage(SYSTEM_REQ, data, 7, 0, 0, 0, 1000);
}

int BleProtocol::Provision(uint16_t deviceAddr)
{
	LOGD("Provision");
	uint8_t dataRsp[100];
	int lenRsp;
	typedef struct
	{
		uint8_t opcode;
		uint8_t netKey[16];
		uint8_t rev[3];
		uint8_t magic[4];
		uint8_t addr[2];
	} provision_message_t;
	provision_message_t provision_message;
	memset(&provision_message, 0x00, sizeof(provision_message));
	provision_message.opcode = HCI_GATEWAY_CMD_SET_NODE_PARA;
	for (int i = 0; i < 16; i++)
	{
		provision_message.netKey[i] = netKey[i];
	}
	provision_message.magic[0] = 0x11;
	provision_message.magic[1] = 0x22;
	provision_message.magic[2] = 0x33;
	provision_message.magic[3] = 0x44;
	provision_message.addr[0] = deviceAddr & 0xFF;
	provision_message.addr[1] = (deviceAddr >> 8) & 0xFF;
	int rs = SendMessage(SYSTEM_REQ, (uint8_t *)&provision_message, 26, HCI_GATEWAY_CMD_PROVISION_EVT, dataRsp, &lenRsp, 15000);
	if (rs == 0)
	{
		typedef struct
		{
			uint8_t status;
			uint8_t data[24];
		} provision_rsp_message_t;
		provision_rsp_message_t *provision_rsp_message = (provision_rsp_message_t *)dataRsp;
		if (provision_rsp_message->status)
		{
			LOGD("Provision OK");
			return 0;
		}
		else
		{
			LOGW("Must hard reset Ble module");
		}
	}
	LOGW("Provision err");
	return -1;
}

int BleProtocol::BindingAll()
{
	LOGD("BindingAll");
	uint8_t dataRsp[100];
	int lenRsp;
	typedef struct
	{
		uint8_t opcode;
		uint8_t rev[3];
		uint8_t appKey[16];
	} binding_all_message_t;
	binding_all_message_t binding_all_message;
	memset(&binding_all_message, 0x00, sizeof(binding_all_message));
	binding_all_message.opcode = HCI_GATEWAY_CMD_START_KEYBIND;
	for (int i = 0; i < 16; i++)
	{
		binding_all_message.appKey[i] = appKey[i];
	}
	int rs = SendMessage(SYSTEM_REQ, (uint8_t *)&binding_all_message, 20, HCI_GATEWAY_CMD_KEY_BIND_EVT, dataRsp, &lenRsp, 30000);
	if (rs == 0)
	{
		if (lenRsp == 1 && dataRsp[0] == 1)
		{
			LOGD("BindingAll OK");
			return 0;
		}
	}
	LOGW("BindingAll err");
	return -1;
}

int BleProtocol::SetGwAddr(uint16_t devAddr, uint16_t gwAddrSet)
{
	LOGD("SetGwAddr");
	uint8_t dataRsp[100];
	int lenRsp;
	uint8_t setGwAddrHeader[] = {0xe1, 0x11, 0x02, 0x02, 0x00};
	typedef struct
	{
		uint8_t rev[4];
		uint16_t header;
		uint16_t devAddr;
		uint8_t data[13];
	} set_gw_addr_message_t;
	set_gw_addr_message_t set_gw_addr_message;
	memset(&set_gw_addr_message, 0x00, sizeof(set_gw_addr_message));
	set_gw_addr_message.header = 0;
	set_gw_addr_message.devAddr = devAddr;
	set_gw_addr_message.data[0] = 0xE0;
	set_gw_addr_message.data[1] = 0x11;
	set_gw_addr_message.data[2] = 0x02;
	set_gw_addr_message.data[3] = 0xE1;
	set_gw_addr_message.data[4] = 0x00;
	set_gw_addr_message.data[5] = 0x02;
	set_gw_addr_message.data[6] = 0x00;
	set_gw_addr_message.data[7] = 0x01;
	int rs = SendMessage(APP_REQ, (uint8_t *)&set_gw_addr_message, 21, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 5000, setGwAddrHeader, 4, 5);
	if (rs == 0)
	{
		typedef struct
		{
			uint16_t devAddr;
			uint16_t gwAddr;
			uint8_t opcode[3];
			uint8_t header[2];
			uint8_t rev[6];
		} set_gw_addr_rsp_message_t;
		set_gw_addr_rsp_message_t *set_gw_addr_rsp_message = (set_gw_addr_rsp_message_t *)dataRsp;
		if (set_gw_addr_rsp_message->devAddr == devAddr)
		{
			if (set_gw_addr_rsp_message->opcode[0] == 0xE1 && set_gw_addr_rsp_message->opcode[1] == 0x11 && set_gw_addr_rsp_message->opcode[2] == 0x02 && set_gw_addr_rsp_message->header[0] == 0x02 && set_gw_addr_rsp_message->header[1] == 0x00)
			{
				LOGD("SetGwAddr OK");
				return 0;
			}
		}
	}
	LOGW("SetGwAddr err");
	return -1;
}

/**
 * @brief Gen Ble security key (6 bytes)
 *
 * @param mac mac of destination device
 * @param devAddr unicast addr of destination device
 * @param out out buffer to write key
 */
static void genSecurityKey(uint8_t *mac, uint16_t devAddr, uint8_t *out)
{
	AES aes(AESKeyLength::AES_128);
	memcpy(plaintext + 8, mac, 6);
	memcpy(plaintext + 14, (uint8_t *)&devAddr, 2);
	for (int n = 0; n < 16; n++)
	{
		printf("%02x ", plaintext[n]);
	}
	printf("\n");
	unsigned char *outAes = aes.EncryptECB(plaintext, 32, keyAes);
	for (int j = 0; j < 32; j++)
	{
		printf("%02x ", outAes[j]);
	}
	printf("\n");
	for (int i = 0; i < 6; i++)
	{
		out[i] = outAes[i + 10];
	}
	free(outAes);
}

int BleProtocol::GetDeviceType(uint8_t *mac, uint16_t devAddr, uint32_t &deviceType, uint16_t &deviceVersion)
{
	LOGD("GetDeviceType");
	uint8_t dataRsp[100];
	int lenRsp;
	uint8_t checkTypeHeader[] = {0xe1, 0x11, 0x02, 0x03, 0x00};
	typedef struct
	{
		uint8_t rev[4];
		uint16_t header;
		uint16_t addr;
		uint8_t data[13];
	} check_type_message_t;
	check_type_message_t check_type_message;
	memset(&check_type_message, 0x00, sizeof(check_type_message));
	check_type_message.header = 0;
	check_type_message.addr = devAddr;
	check_type_message.data[0] = 0xE0;
	check_type_message.data[1] = 0x11;
	check_type_message.data[2] = 0x02;
	check_type_message.data[3] = 0xE1;
	check_type_message.data[4] = 0x00;
	check_type_message.data[5] = 0x03;
	genSecurityKey(mac, devAddr, &check_type_message.data[7]);
	int rs = SendMessage(APP_REQ, (uint8_t *)&check_type_message, 21, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 5000, checkTypeHeader, 4, 5);
	if (rs == 0)
	{
		typedef struct
		{
			uint16_t devAddr;
			uint16_t gwAddr;
			uint8_t opcode[3];
			uint8_t header[2];
			uint8_t deviceType[3];
			uint8_t magic;
			uint8_t version[2];
		} check_type_rsp_message_t;
		check_type_rsp_message_t *check_type_rsp_message = (check_type_rsp_message_t *)dataRsp;
		if (check_type_rsp_message->devAddr == devAddr)
		{
			if (check_type_rsp_message->opcode[0] == 0xE1 && check_type_rsp_message->opcode[1] == 0x11 && check_type_rsp_message->opcode[2] == 0x02 && check_type_rsp_message->header[0] == 0x03 && check_type_rsp_message->header[1] == 0x00)
			{
				deviceType = (check_type_rsp_message->deviceType[0] << 16) | (check_type_rsp_message->deviceType[1] << 8) | check_type_rsp_message->deviceType[2];
				deviceVersion = (check_type_rsp_message->version[0] << 8) | (check_type_rsp_message->version[1]);
				LOGD("GetDeviceType OK, deviceType: 0x%04X, version: %d", deviceType, deviceVersion);
				return 0;
			}
		}
	}
	LOGW("GetDeviceType err");
	return -1;
}

int BleProtocol::ResetDev(uint16_t devAddr)
{
	LOGD("Reset dev addr: 0x%04X", devAddr);
	uint8_t dataRsp[100];
	int lenRsp;
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
	} reset_message_t;
	reset_message_t reset_message = {0};
	memset(&reset_message, 0x00, sizeof(reset_message));
	uint8_t resetHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x80, 0x4a};
	reset_message.addr = devAddr;
	reset_message.opcode = 0x4980;
	int rs = SendMessage(APP_REQ, (uint8_t *)&reset_message, 10, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, resetHeader, 0, 6);
	if (rs == 0)
	{
		return 0;
	}
	return -1;
}

int BleProtocol::SendOnlineCheck(uint16_t devAddr)
{
	LOGV("SendOnlineCheck addr: 0x%04X", devAddr);
	return GetOnoffLight(devAddr);
}

int BleProtocol::SetOnOffLight(uint16_t devAddr, uint8_t onoff, uint16_t transition, bool ack)
{
	LOGD("Set OnOff addr: 0x%04X value %d", devAddr, onoff);
	uint8_t dataRsp[100];
	int lenRsp;
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint8_t onoff;
		uint8_t rev2;
		uint8_t transition[2];
	} onoff_message_t;
	onoff_message_t onoff_message = {0};
	memset(&onoff_message, 0x00, sizeof(onoff_message));
	if (ack)
	{
		uint8_t turnOnOffHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x04, onoff};
		onoff_message.addr = devAddr;
		onoff_message.opcode = 0x0282;
		onoff_message.onoff = onoff;
		onoff_message.rev2 = 0;
		onoff_message.transition[0] = transition & 0xFF;
		onoff_message.transition[1] = (transition >> 8) & 0xFF;
		int rs = SendMessage(APP_REQ, (uint8_t *)&onoff_message, 14, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, turnOnOffHeader, 0, 7);
		if (rs == 0)
		{
			typedef struct
			{
				uint16_t devAddr;
				uint16_t gwAddr;
				uint16_t opcode;
				uint8_t data[3];
			} onoff_rsp_message_t;
			onoff_rsp_message_t *onoff_rsp_message = (onoff_rsp_message_t *)dataRsp;
			if (onoff_rsp_message->opcode == 0x0482)
			{
				if (lenRsp == 7)
				{
					if (onoff == onoff_rsp_message->data[0])
						return 0;
				}
				else
				{
					if (onoff == onoff_rsp_message->data[1])
						return 0;
				}
				LOGW("Onoff resp state not match with input control");
			}
		}
	}
	else
	{
		onoff_message.addr = devAddr;
		onoff_message.opcode = 0x0382;
		onoff_message.onoff = onoff;
		onoff_message.rev2 = 0;
		onoff_message.transition[0] = transition & 0xFF;
		onoff_message.transition[1] = (transition >> 8) & 0xFF;
		int rs = SendMessage(APP_REQ, (uint8_t *)&onoff_message, 14, 0, dataRsp, &lenRsp, 1000);
		if (rs == 0)
		{
			return 0;
		}
	}
	LOGW("SetOnOff err");
	return -1;
}

int BleProtocol::GetOnoffLight(uint16_t devAddr)
{
	LOGD("Get OnOff addr: 0x%04X", devAddr);
	uint8_t dataRsp[100];
	int lenRsp;
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
	} onoff_message_t;
	onoff_message_t onoff_message = {0};
	memset(&onoff_message, 0x00, sizeof(onoff_message));
	uint8_t getOnOffHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x04};
	onoff_message.addr = devAddr;
	onoff_message.opcode = G_ONOFF_GET;
	int rs = SendMessage(APP_REQ, (uint8_t *)&onoff_message, 10, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, getOnOffHeader, 0, 6);
	if (rs == 0)
	{
		typedef struct
		{
			uint16_t devAddr;
			uint16_t gwAddr;
			uint16_t opcode;
			uint8_t data[3];
		} onoff_rsp_message_t;
		onoff_rsp_message_t *onoff_rsp_message = (onoff_rsp_message_t *)dataRsp;
		if (onoff_rsp_message->opcode == G_ONOFF_STATUS)
		{
			return 0;
		}
	}

	LOGW("GetOnOff err");
	return -1;
}

int BleProtocol::SetDimmingLight(uint16_t devAddr, uint16_t dim, uint16_t transition, bool ack)
{
	LOGD("Dimming addr: 0x%04X value %d", devAddr, dim);
	uint8_t dataRsp[100];
	int lenRsp;
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint16_t dim;
		uint8_t offset;
		uint8_t transition[2];
	} dim_message_t;
	dim_message_t dim_message;
	memset(&dim_message, 0x00, sizeof(dim_message));
	if (ack)
	{
		uint8_t dimmingHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x4E};

		dim_message.addr = devAddr;
		dim_message.opcode = 0x4C82;
		dim_message.dim = dim;
		dim_message.offset = 0;
		dim_message.transition[0] = transition & 0xFF;
		dim_message.transition[1] = (transition >> 8) & 0xFF;
		int rs = SendMessage(APP_REQ, (uint8_t *)&dim_message, 15, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, dimmingHeader, 0, 6);
		if (rs == 0)
		{
			typedef struct
			{
				uint16_t devAddr;
				uint16_t gwAddr;
				uint16_t opcode;
				uint8_t data[8];
			} dim_rsp_message_t;
			dim_rsp_message_t *dim_rsp_message = (dim_rsp_message_t *)dataRsp;
			if (lenRsp == 8)
			{
				if ((dim_rsp_message->data[0] | dim_rsp_message->data[1] << 8) == dim)
				{
					return 0;
				}
			}
			else if (lenRsp > 8)
			{
				if ((dim_rsp_message->data[2] | dim_rsp_message->data[3] << 8) == dim)
				{
					return 0;
				}
			}
		}
	}
	else
	{
		dim_message.addr = devAddr;
		dim_message.opcode = 0x4d82;
		dim_message.dim = dim;
		dim_message.offset = 0;
		dim_message.transition[0] = transition & 0xFF;
		dim_message.transition[1] = (transition >> 8) & 0xFF;
		int rs = SendMessage(APP_REQ, (uint8_t *)&dim_message, 15, 0, dataRsp, &lenRsp, 1000);
		if (rs == 0)
		{
			return 0;
		}
	}
	LOGW("Dimming err");
	return -1;
}

int BleProtocol::SetCctLight(uint16_t devAddr, uint16_t cct, uint16_t transition, bool ack)
{
	LOGD("Set Cct addr: 0x%04X value %d", devAddr, cct);
	uint8_t dataRsp[100];
	int lenRsp;
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint16_t cct;
		uint8_t offset[3];
		uint8_t transition[2];
	} cct_message_t;
	cct_message_t cct_message;
	memset(&cct_message, 0x00, sizeof(cct_message));
	if (ack)
	{
		uint8_t cctHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 00, 0x82, 0x66};
		cct_message.addr = devAddr;
		cct_message.opcode = 0x6482;
		cct_message.cct = cct;
		for (int count = 0; count < 3; count++)
		{
			cct_message.offset[count] = 0;
		}
		cct_message.transition[0] = transition & 0xFF;
		cct_message.transition[1] = (transition >> 8) & 0xFF;

		int rs = SendMessage(APP_REQ, (uint8_t *)&cct_message, 17, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, cctHeader, 0, 6);
		if (rs == 0)
		{
			typedef struct
			{
				uint16_t devAddr;
				uint16_t gwAddr;
				uint16_t opcode;
				uint8_t data[8];
			} cct_rsp_message_t;
			cct_rsp_message_t *cct_rsp_message = (cct_rsp_message_t *)dataRsp;
			if (lenRsp == 10)
			{
				if (cct == (cct_rsp_message->data[0] | (cct_rsp_message->data[1] << 8)))
				{
					return 0;
				}
			}
			else if (lenRsp > 10)
			{
				if (cct == (cct_rsp_message->data[4] | (cct_rsp_message->data[5] << 8)))
				{
					return 0;
				}
			}
		}
	}
	else if (ack == false)
	{
		cct_message.addr = devAddr;
		cct_message.opcode = 0x6582;
		cct_message.cct = cct;
		for (int count = 0; count < 3; count++)
		{
			cct_message.offset[count] = 0;
		}
		cct_message.transition[0] = transition & 0xFF;
		cct_message.transition[1] = (transition >> 8) & 0xFF;

		int rs = SendMessage(APP_REQ, (uint8_t *)&cct_message, 10, 0, dataRsp, &lenRsp, 1000);
		if (rs == 0)
		{
			return 0;
		}
	}
	LOGW("Cct err");
	return -1;
}

int BleProtocol::SetHSLLight(uint16_t devAddr, uint16_t H, uint16_t S, uint16_t L, uint16_t transition, bool ack)
{
	LOGD("HSL addr: 0x%04X value HSL: %d-%d-%d", devAddr, H, S, L);
	uint8_t dataRsp[100];
	int lenRsp;
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint16_t l;
		uint16_t h;
		uint16_t s;
		uint8_t offset;
		uint8_t transition[2];
	} hsl_message_t;
	hsl_message_t hsl_message = {0};
	memset(&hsl_message, 0x00, sizeof(hsl_message));
	if (ack)
	{
		uint8_t hslHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x78};
		hsl_message.addr = devAddr;
		hsl_message.opcode = 0x7682;
		hsl_message.l = L;
		hsl_message.h = H;
		hsl_message.s = S;
		hsl_message.offset = 0;
		hsl_message.transition[0] = transition & 0xFF;
		hsl_message.transition[1] = (transition >> 8) & 0xFF;
		int rs = SendMessage(APP_REQ, (uint8_t *)&hsl_message, 19, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, hslHeader, 0, 6);
		if (rs == 0)
		{
			typedef struct
			{
				uint16_t devAddr;
				uint16_t gwAddr;
				uint16_t opcode;
				uint16_t l;
				uint16_t h;
				uint16_t s;
			} hsl_rsp_message_t;
			hsl_rsp_message_t *hsl_rsp_message = (hsl_rsp_message_t *)dataRsp;
			if (hsl_rsp_message->h == H && hsl_rsp_message->l == L && hsl_rsp_message->s == S)
			{
				return 0;
			}
			LOGW("hsl resp state not match with input control");
		}
	}
	else
	{
		hsl_message.addr = devAddr;
		hsl_message.opcode = 0x7782;
		hsl_message.l = L;
		hsl_message.h = H;
		hsl_message.s = S;
		hsl_message.offset = 0;
		hsl_message.transition[0] = transition & 0xFF;
		hsl_message.transition[1] = (transition >> 8) & 0xFF;
		int rs = SendMessage(APP_REQ, (uint8_t *)&hsl_message, 19, 0, dataRsp, &lenRsp, 1000);
		if (rs == 0)
		{
			return 0;
		}
	}
	LOGW("Set hsl err");
	return -1;
}

int BleProtocol::SetCctDimLight(uint16_t devAddr, uint16_t cct, uint16_t dim, uint16_t transition, bool ack)
{
	LOGD("Set Dim cct addr: 0x%04X", devAddr);
	uint8_t dataRsp[100];
	int lenRsp;
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint16_t dim;
		uint16_t cct;
		uint8_t offset;
		uint8_t transition[2];
	} dimcct_message_t;
	dimcct_message_t dimcct_message = {0};
	memset(&dimcct_message, 0x00, sizeof(dimcct_message));
	if (ack)
	{
		uint8_t dimcctHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x60};
		dimcct_message.addr = devAddr;
		dimcct_message.opcode = 0x5e82;
		dimcct_message.dim = dim;
		dimcct_message.cct = cct;
		dimcct_message.offset = 0;
		dimcct_message.transition[0] = transition & 0xFF;
		dimcct_message.transition[1] = (transition >> 8) & 0xFF;
		int rs = SendMessage(APP_REQ, (uint8_t *)&dimcct_message, 19, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, dimcctHeader, 0, 6);
		if (rs == 0)
		{
			typedef struct
			{
				uint16_t devAddr;
				uint16_t gwAddr;
				uint16_t opcode;
				uint16_t dim;
				uint16_t cct;
			} dimcct_rsp_message_t;
			dimcct_rsp_message_t *dimcct_rsp_message = (dimcct_rsp_message_t *)dataRsp;
			if (dimcct_rsp_message->dim == dim && dimcct_rsp_message->cct == cct)
			{
				return 0;
			}
			LOGW("dim cct resp state not match with input control");
		}
	}
	else
	{
		dimcct_message.addr = devAddr;
		dimcct_message.opcode = 0x5f82;
		dimcct_message.dim = dim;
		dimcct_message.cct = cct;
		dimcct_message.offset = 0;
		dimcct_message.transition[0] = transition & 0xFF;
		dimcct_message.transition[1] = (transition >> 8) & 0xFF;
		int rs = SendMessage(APP_REQ, (uint8_t *)&dimcct_message, 19, 0, dataRsp, &lenRsp, 1000);
		if (rs == 0)
		{
			return 0;
		}
	}
	LOGW("Set dim cct err");
	return -1;
}

int BleProtocol::AddDev2Group(uint16_t devAddr, uint16_t element, uint16_t group)
{
	LOGD("Add dev addr: 0x%04X  with element: 0x%04x to group: 0x%04X", devAddr, element, group);
	uint8_t dataRsp[100];
	int lenRsp;
	uint8_t addGroupHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x80, 0x1f};
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint16_t element;
		uint16_t group;
		uint8_t offset[2];
	} addgroup_message_t;
	addgroup_message_t addgroup_message = {0};
	memset(&addgroup_message, 0x00, sizeof(addgroup_message));
	addgroup_message.addr = devAddr;
	addgroup_message.opcode = 0x1b80;
	addgroup_message.element = element;
	addgroup_message.group = group;
	addgroup_message.offset[0] = 0;
	addgroup_message.offset[1] = 0x10;
	int rs = SendMessage(APP_REQ, (uint8_t *)&addgroup_message, 16, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, addGroupHeader, 0, 6);
	if (rs == 0)
	{
		typedef struct
		{
			uint16_t devAddr;
			uint16_t gwAddr;
			uint16_t opcode;
			uint8_t offset;
			uint8_t element[2];
			uint8_t group[2];
		} addgroup_rsp_message_t;
		addgroup_rsp_message_t *addgroup_rsp_message = (addgroup_rsp_message_t *)dataRsp;
		LOGD("adr : 0x%04x, gw:0x%04x, opcode:0x%04x, offset:0x%02x, element: 0x%04x, group: 0x%04x", addgroup_rsp_message->devAddr, addgroup_rsp_message->gwAddr, addgroup_rsp_message->opcode, addgroup_rsp_message->offset, addgroup_rsp_message->element[0] | (addgroup_rsp_message->element[1] << 8), addgroup_rsp_message->group[0] | (addgroup_rsp_message->group[1] << 8));
		if (element == (addgroup_rsp_message->element[0] | (addgroup_rsp_message->element[1] << 8)) && ((addgroup_rsp_message->group[0] | (addgroup_rsp_message->group[1] << 8)) == group))
		{
			return 0;
		}
		LOGW("add group resp state not match with input control");
	}
	LOGW("Add group err");
	return -1;
}

int BleProtocol::DelDev2Group(uint16_t devAddr, uint16_t element, uint16_t group)
{
	LOGD("Del dev addr: 0x%04X  with element: 0x%04x to group: 0x%04X", devAddr, element, group);
	uint8_t dataRsp[100];
	int lenRsp;
	uint8_t delGroupHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x80, 0x1f};
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint16_t element;
		uint16_t group;
		uint8_t offset[2];
	} delgroup_message_t;
	delgroup_message_t delgroup_message = {0};
	memset(&delgroup_message, 0x00, sizeof(delgroup_message));
	delgroup_message.addr = devAddr;
	delgroup_message.opcode = 0x1c80;
	delgroup_message.element = element;
	delgroup_message.group = group;
	delgroup_message.offset[0] = 0;
	delgroup_message.offset[1] = 0x10;
	int rs = SendMessage(APP_REQ, (uint8_t *)&delgroup_message, 16, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, delGroupHeader, 0, 6);
	if (rs == 0)
	{
		return 0;
	}
	LOGW("Del group err");
	return -1;
}

int BleProtocol::SetSceneLights(uint16_t devAddr, uint16_t scene, uint8_t modeRgb)
{
	LOGD("Set scene addr: 0x%04X to scene: 0x%04X", devAddr, scene);
	uint8_t dataRsp[100];
	int lenRsp;
	uint8_t setSceneHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x45};
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint16_t scene;
		uint8_t modeRgb;
		uint8_t offset[2];
	} setscene_message_t;
	setscene_message_t setscene_message = {0};
	memset(&setscene_message, 0x00, sizeof(setscene_message));
	setscene_message.addr = devAddr;
	setscene_message.opcode = 0x4682;
	setscene_message.scene = scene;
	setscene_message.modeRgb = modeRgb;
	setscene_message.offset[0] = 0;
	setscene_message.offset[1] = 0;
	int rs = SendMessage(APP_REQ, (uint8_t *)&setscene_message, 15, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, setSceneHeader, 0, 6);
	if (rs == 0)
	{
		typedef struct
		{
			uint16_t devAddr;
			uint16_t gwAddr;
			uint16_t opcode;
			uint8_t offset;
			uint8_t scene[2];
		} setscene_rsp_message_t;
		setscene_rsp_message_t *setscene_rsp_message = (setscene_rsp_message_t *)dataRsp;
		if ((setscene_rsp_message->scene[0] | (setscene_rsp_message->scene[1] << 8)) == scene)
		{
			return 0;
		}
		else
		{
			LOGW("set scene resp state not match with input control");
		}
	}
	LOGW("Set scene err");
	return -1;
}

// TODO: BelProtocol DelScene
int BleProtocol::DelSceneLights(uint16_t devAddr, uint16_t scene)
{
	LOGD("Del scene addr: 0x%04X to scene: 0x%04X", devAddr, scene);
	uint8_t dataRsp[100];
	int lenRsp;
	uint8_t delSceneHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x45};
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint16_t scene;
	} delscene_message_t;
	delscene_message_t delscene_message = {0};
	memset(&delscene_message, 0x00, sizeof(delscene_message));
	delscene_message.addr = devAddr;
	delscene_message.opcode = 0x9e82;
	delscene_message.scene = scene;
	int rs = SendMessage(APP_REQ, (uint8_t *)&delscene_message, 12, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, delSceneHeader, 0, 6);
	if (rs == 0)
	{
		return 0;
	}
	LOGW("Del scene err");
	return -1;
}

// TODO: BelProtocol ActiveScene
// add delay time
int BleProtocol::CallScene(uint16_t devAddr, uint16_t scene, uint16_t transition, bool ack, int delayTime)
{
	LOGD("Call scene: 0x%04X", scene);
	uint8_t dataRsp[100];
	int lenRsp;
	uint8_t callSceneHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x5e, 0x00};
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint16_t scene;
		uint8_t offset;
		uint8_t transition[2];
	} callscene_message_t;
	callscene_message_t callscene_message = {0};
	memset(&callscene_message, 0x00, sizeof(callscene_message));
	if (ack)
	{
		callscene_message.addr = devAddr;
		callscene_message.opcode = 0x4282;
		callscene_message.scene = scene;
		callscene_message.offset = 0;
		callscene_message.transition[0] = transition;
		callscene_message.transition[1] = (transition >> 8) & 0xFF;
		int rs = SendMessage(APP_REQ, (uint8_t *)&callscene_message, 15, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, callSceneHeader, 0, 6);
		if (rs == 0)
		{
			typedef struct
			{
				uint16_t devAddr;
				uint16_t gwAddr;
				uint16_t opcode;
				uint8_t data[8];
			} callscene_rsp_message_t;
			callscene_rsp_message_t *callscene_rsp_message = (callscene_rsp_message_t *)dataRsp;
			if (lenRsp == 11 || lenRsp == 13)
			{
				if (scene == (callscene_rsp_message->data[2] | (callscene_rsp_message->data[3] << 8)))
				{
					return 0;
				}
				else
				{
					LOGW("call scene resp state not match with input control");
				}
			}
			else
			{
				if (scene == (callscene_rsp_message->data[0] | (callscene_rsp_message->data[1] << 8)))
				{
					return 0;
				}
				else
				{
					LOGW("call scene resp state not match with input control");
				}
			}
		}
	}
	else
	{
		callscene_message.addr = devAddr;
		callscene_message.opcode = 0x4382;
		callscene_message.scene = scene;
		callscene_message.offset = 0;
		callscene_message.transition[0] = transition;
		callscene_message.transition[1] = (transition >> 8) & 0xFF;
		int rs = SendMessage(APP_REQ, (uint8_t *)&callscene_message, 15, 0, dataRsp, &lenRsp, 1000);
		if (rs == 0)
		{
			return 0;
		}
	}

	LOGW("Call scene err");
	return -1;
}

int BleProtocol::CallModeRgb(uint16_t devAddr, uint8_t modeRgb)
{
	LOGD("Call modeRgb: %d, addr: 0x%04X ", modeRgb, devAddr);
	uint8_t dataRsp[100];
	int lenRsp;
	uint8_t modeRgbHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x52};
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint16_t header;
		uint8_t mode;
	} modergb_message_t;
	modergb_message_t modergb_message = {0};
	memset(&modergb_message, 0x00, sizeof(modergb_message));
	modergb_message.addr = devAddr;
	modergb_message.opcode = 0x5082;
	modergb_message.header = 0x0919;
	modergb_message.mode = modeRgb;
	int rs = SendMessage(APP_REQ, (uint8_t *)&modergb_message, 14, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, modeRgbHeader, 0, 6);
	if (rs == 0)
	{
		typedef struct
		{
			uint16_t devAddr;
			uint16_t gwAddr;
			uint16_t opcode;
			uint16_t header;
			uint8_t mode;
		} modergb_rsp_message_t;
		modergb_rsp_message_t *modergb_rsp_message = (modergb_rsp_message_t *)dataRsp;
		if (modergb_rsp_message->header == 0x0919)
		{
			if (modergb_rsp_message->mode == modeRgb)
			{
				return 0;
			}
			LOGW("call mode rgb resp state not match with input control");
		}
	}
	LOGW("call mode rgb err");
	return -1;
}

int BleProtocol::UpdateLights(uint16_t devAddr)
{
	LOGD("Update lights addr: 0x%04X ", devAddr);
	uint8_t dataRsp[100];
	int lenRsp;
	uint8_t updateHeader[] = {(uint8_t)(devAddr & 0xFF), (uint8_t)((devAddr >> 8) & 0xFF), 1, 0, 0x82, 0x52};
	typedef struct
	{
		uint8_t rev[6];
		uint16_t addr;
		uint16_t opcode;
		uint8_t header;
		uint8_t data[7];
	} update_message_t;
	update_message_t update_message = {0};
	memset(&update_message, 0x00, sizeof(update_message));
	update_message.addr = devAddr;
	update_message.opcode = 0x5082;
	update_message.header = 0x02;
	int rs = SendMessage(APP_REQ, (uint8_t *)&update_message, 12, HCI_GATEWAY_RSP_OP_CODE, dataRsp, &lenRsp, 1000, updateHeader, 0, 6);
	if (rs == 0)
	{
		typedef struct
		{
			uint16_t devAddr;
			uint16_t gwAddr;
			uint16_t opcode;
			uint8_t header;
		} update_rsp_message_t;
		update_rsp_message_t *update_rsp_message = (update_rsp_message_t *)dataRsp;
		if (update_rsp_message->header == 0x02)
		{
			return 0;
		}
		LOGW("update lights resp state not match with input control");
	}
	LOGW("update lights mode rgb err");
	return -1;
}
