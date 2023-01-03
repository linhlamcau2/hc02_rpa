#pragma once

#include <stdint.h>
#include <vector>
#include <Uart.h>
#include <atomic>

#define SYSTEM_REQ 0xFFE9
#define APP_REQ 0xFFE8
#define RAL_MAGIC 0x0428

enum
{
	// send cmd part
	HCI_GATEWAY_CMD_START = 0x00,
	HCI_GATEWAY_CMD_STOP = 0x01,
	HCI_GATEWAY_CMD_RESET = 0x02,
	HCI_GATEWAY_CMD_CLEAR_NODE_INFO = 0x06,
	HCI_GATEWAY_CMD_SET_ADV_FILTER = 0x08,
	HCI_GATEWAY_CMD_SET_PRO_PARA = 0x09,
	HCI_GATEWAY_CMD_SET_NODE_PARA = 0x0a,
	HCI_GATEWAY_CMD_START_KEYBIND = 0x0b,
	HCI_GATEWAY_CMD_GET_PRO_SELF_STS = 0x0c,
	HCI_GATEWAY_CMD_SET_DEV_KEY = 0x0d,
	HCI_GATEWAY_CMD_GET_SNO = 0x0e,
	HCI_GATEWAY_CMD_SET_SNO = 0x0f,
	HCI_GATEWAY_CMD_GET_UUID_MAC = 0x10,
	HCI_GATEWAY_CMD_DEL_VC_NODE_INFO = 0x11,
	HCI_GATEWAY_CMD_SEND_VC_NODE_INFO = 0x12,

	// rsp cmd part
	HCI_GATEWAY_RSP_UNICAST = 0x80,
	HCI_GATEWAY_RSP_OP_CODE = 0X81,
	HCI_GATEWAY_KEY_BIND_RSP = 0x82,
	HCI_GATEWAY_CMD_STATIC_OOB_RSP = 0x87, // HCI send back the static oob information
	HCI_GATEWAY_CMD_UPDATE_MAC = 0x88,
	HCI_GATEWAY_CMD_PROVISION_EVT = 0x89,
	HCI_GATEWAY_CMD_KEY_BIND_EVT = 0x8a,
	HCI_GATEWAY_CMD_PRO_STS_RSP = 0x8b,
	HCI_GATEWAY_CMD_SEND_ELE_CNT = 0x8c,
	HCI_GATEWAY_CMD_SEND_NODE_INFO = 0x8d,
	HCI_GATEWAY_CMD_SEND_CPS_INFO = 0x8e,
	HCI_GATEWAY_CMD_HEARTBEAT = 0x8f,
	HCI_GATEWAY_CMD_SEND_MESH_OTA_STS = 0x98,
	HCI_GATEWAY_CMD_SEND_UUID = 0x99,
	HCI_GATEWAY_CMD_SEND_IVI = 0x9a,
	HCI_GATEWAY_CMD_SEND_SRC_CMD = 0x9c,
	HCI_GATEWAY_CMD_SEND_SNO_RSP = 0xa0,
	HCI_GATEWAY_CMD_SEND = 0xb1,
	HCI_GATEWAY_DEV_RSP = 0xb2,
	HCI_GATEWAY_CMD_LINK_OPEN = 0xb3,
	HCI_GATEWAY_CMD_LINK_CLS = 0xb4,
	HCI_GATEWAY_CMD_SEND_BACK_VC = 0xb5,
	HCI_GATEWAY_CMD_LOG_STRING = 0xb6,
	HCI_GATEWAY_CMD_LOG_BUF = 0xb7,
};

#define CONNECT_DEVICE_TIMEOUT 40 // seconds

using namespace std;

class BleProtocol : public Uart
{
private:
	typedef struct
	{
		uint16_t opcode;
		uint8_t data[100];
	} message_req_st;

	typedef struct
	{
		uint16_t len;
		uint8_t magic;
		uint8_t opcode;
		uint8_t data[];
	} message_rsp_st;

	typedef struct
	{
		bool status;
		uint8_t opcode;
		int *len;
		uint8_t *data;
		uint8_t *compare_data;
		int compare_position;
		int compare_len;
	} message_rsp_list_st;

	typedef struct
	{
		uint8_t uuid[8];
		uint8_t deviceType[4];
		uint16_t fwVersion;
		uint16_t magic;
	} uuid_t;

	typedef struct
	{
		uint8_t mac[6];
		uint8_t len;
		uint8_t header_type;
		uint8_t beacon_type;
		uint8_t uuid[16];
		uint8_t uri_hash[4];
		uint8_t obb_info[2];
		int8_t rssi;
		uint8_t dc[2];
	} scan_device_message_t;

	typedef function<void(scan_device_message_t *scan_device_message)> AddDeviceFunc;
	AddDeviceFunc addDeviceFunc;
	atomic<bool> isAdding;
	scan_device_message_t scanDeviceMessage;

	vector<message_rsp_list_st *> messageRespList;

	// TODO: Add init state
	uint8_t netKey[16];
	uint8_t appKey[16];
	uint8_t gwKey[16];
	uint16_t nextAddr;

	string uuidToStr(uuid_t *uuid);

	void CheckOpcodeException(message_rsp_st *message);
	void OnMessage(unsigned char *data, int len);
	int SendMessage(uint16_t opReq, uint8_t *dataReq, int lenReq, uint8_t opRsp, uint8_t *dataRsp, int *lenRsp, uint32_t timeout, uint8_t *compare_data = 0, int compare_position = 0, int compare_len = 0);

public:
	BleProtocol(char *uartPort, int uartBaudrate);
	virtual ~BleProtocol();

	void init();
	int GetAppKey();
	int GetNetKey();
	int SetNetKey();
	int SetGwKey();

	int StartScan();
	int StopScan();
	int ResetFactory();

	void AddDevice(scan_device_message_t *scan_device_message);
	int SelectMac(uint8_t *mac);
	int Provision(uint16_t deviceAddr);
	int BindingAll();
	int SetGwAddr(uint16_t devAddr, uint16_t gwAddr = 0x0002);
	int GetDeviceType(uint8_t *mac, uint16_t devAddr, uint32_t &deviceType, uint16_t deviceVersion);

	int ResetDev(uint16_t devAddr);

	int SetOnOffLight(uint16_t devAddr, uint8_t onoff, uint16_t transition, bool ack);
	int GetOnoffLight(uint16_t devAddr);
	int SetDimmingLight(uint16_t devAddr, uint16_t dim, uint16_t transition, bool ack);
	int GetDimming(uint16_t devAddr);
	int SetCctLight(uint16_t devAddr, uint16_t cct, uint16_t transition, bool ack);
	int GetCct(uint16_t devAddr);
	int SetHSLLight(uint16_t devAddr, uint16_t H, uint16_t S, uint16_t L, uint16_t transition, bool ack);
	int GetHSL(uint16_t devAddr);
	int SetCctDimLight(uint16_t devAddr, uint16_t cct, uint16_t dim, uint16_t transition, bool ack);
	int GetCctDimLight(uint16_t devAddr);

    	//group light
	int AddDev2Group(uint16_t devAddr, uint16_t element, uint16_t group);
	int DelDev2Group(uint16_t devAddr, uint16_t element, uint16_t group);
	
	//scene lights
	int SetSceneLights(uint16_t devAddr, uint16_t scene, uint8_t modeRgb);
	int DelSceneLights(uint16_t devAddr, uint16_t scene);
	int CallSceneLight(uint16_t devAddr, uint16_t scene, uint16_t transition, bool ack);
	int CallModeRgb(uint16_t devAddr, uint8_t modeRgb);
	
	//update status lights
	int UpdateLights(uint16_t devAddr);
};

extern BleProtocol *bleProtocol;
