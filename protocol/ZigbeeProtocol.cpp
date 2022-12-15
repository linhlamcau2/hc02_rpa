#ifdef CONFIG_ENABLE_ZIGBEE

#include "ZigbeeProtocol.h"
#include <stdlib.h>
#include <thread>
#include <functional>
#include <byteswap.h>
#include <Log.h>
#include <Util.h>
#include <string.h>
#include <algorithm>
#include <Db.h>
#include "DeviceZigbee.h"

#include "zigbee/cluster/basic/ClusterBasic.h"
#include "zigbee/ZigbeeDataTypes.h"

ZigbeeProtocol *zigbeeProtocol = NULL;

ZigbeeProtocol::ZigbeeProtocol(char *uartPort, int uartBaudrate) : Uart(uartPort, 100000)
{
	if (Open(uartBaudrate) < 0)
	{
		LOGE("Open uart error")
		exit(1);
	}
}

ZigbeeProtocol::~ZigbeeProtocol()
{
}

void ZigbeeProtocol::init()
{
	RegisterCmdCallback(ZBHCI_CMD_NODES_DEV_ANNCE_IND, bind(&ZigbeeProtocol::OnDeviceAnnounce, this, placeholders::_1, placeholders::_2));
	RegisterCmdCallback(ZBHCI_CMD_ZCL_REPORT_MSG_RCV, bind(&ZigbeeProtocol::OnReportAttribute, this, placeholders::_1, placeholders::_2));
	RegisterCmdCallback(ZBHCI_CMD_ZCL_ATTR_READ_RSP, bind(&ZigbeeProtocol::OnReadAttributeResp, this, placeholders::_1, placeholders::_2));
}

int ZigbeeProtocol::RegisterCmdCallback(uint16_t type, OnCmdCallbackFunc onCmdCallbackFunc)
{
	LOGD("RegisterCmd type: 0x%04X", type);
	onCmdCallbackFuncList[type] = onCmdCallbackFunc;
	return 0;
}

static uint8_t checCrC(uint16_t type, uint16_t len, uint8_t *payload)
{
	uint8_t crc8 = (type >> 0) & 0xff;
	crc8 ^= (type >> 8) & 0xff;
	crc8 ^= (len >> 0) & 0xff;
	crc8 ^= (len >> 8) & 0xff;
	for (int i = 0; i < len; i++)
	{
		crc8 ^= payload[i];
	}
	return crc8;
}

void ZigbeeProtocol::CheckOpcodeException(message_rsp_st *message_rsp)
{
	LOGD("CheckOpcodeException");
	uint16_t type = bswap_16(message_rsp->type);
	uint16_t len = bswap_16(message_rsp->len);
	if (onCmdCallbackFuncList.find(type) != onCmdCallbackFuncList.end())
	{
		OnCmdCallbackFunc onCmdCallbackFunc = onCmdCallbackFuncList[type];
		int rs = onCmdCallbackFunc(message_rsp->payload, len);
		if (rs == 0)
		{
			LOGD("onCmdCallbackFunc OK");
		}
		else
		{
			LOGW("onCmdCallbackFunc rs: %d", rs);
		}
	}
	else
	{
		LOGW("type not registed: 0x%04X", type);
	}
}

void ZigbeeProtocol::OnMessage(unsigned char *data, int len)
{
	LOGD("OnMessage len: %d", len);
	uint8_t *message = data;
	int lenRemain = len;
	message_rsp_st *message_rsp = NULL;
	while (lenRemain >= 7 && message[0] == MESSAGE_HEADER)
	{
		message_rsp = (message_rsp_st *)(message + 1);
		uint16_t type = bswap_16(message_rsp->type);
		uint16_t payloadLen = bswap_16(message_rsp->len);
		if (message_rsp->payload[payloadLen] == MESSAGE_TAIL)
		{
			// LOGD("message_rsp->type: 0x%04X, message_rsp->len: %d", type, payloadLen);
			if (type == ZBHCI_CMD_ACKNOWLEDGE)
			{
				message_acknowledge_st *message_acknowledge = (message_acknowledge_st *)message_rsp->payload;
				uint16_t reqType = bswap_16(message_acknowledge->type);
				// LOGD("OnMessage resp type: 0x%04X, status: %d", reqType, message_acknowledge->status);
				for (auto &messageResp : messageRespList)
				{
					if (reqType == messageResp->reqType)
					{
						messageResp->status = message_acknowledge->status;
						if (messageResp->len)
						{
							*(messageResp->len) = payloadLen;
							if (messageResp->payload)
								memcpy(messageResp->payload, message_rsp->payload, *messageResp->len);
						}
					}
				}
			}
			else
			{
				CheckOpcodeException(message_rsp);
			}
		}
		lenRemain -= payloadLen + 7;
		message += payloadLen + 7;
	}
}

int ZigbeeProtocol::SendMessage(uint16_t opReq, uint8_t *dataReq, int lenReq, uint16_t opRsp, uint8_t *dataRsp, int *lenRsp, uint32_t timeout)
{
	uint8_t buff[128];
	message_rsp_list_st message_rsp_list = {
			.status = 0xFF,
			.reqType = opReq,
			.respType = opRsp,
			.len = lenRsp,
			.payload = dataRsp};
	if (opRsp)
	{
		messageRespList.push_back(&message_rsp_list);
	}

	buff[0] = MESSAGE_HEADER;
	message_req_st *message_req = (message_req_st *)(buff + 1);
	message_req->type = bswap_16(opReq);
	message_req->len = bswap_16(lenReq);
	message_req->crc = checCrC(opReq, lenReq, dataReq);
	for (int i = 0; i < lenReq; i++)
	{
		message_req->payload[i] = dataReq[i];
	}
	message_req->payload[lenReq] = MESSAGE_TAIL;

	Write(buff, 7 + lenReq);

	if (opRsp)
	{

		while (message_rsp_list.status == 0xFF && timeout)
		{
			usleep(1000);
			--timeout;
		}
		messageRespList.erase(remove(messageRespList.begin(), messageRespList.end(), &message_rsp_list), messageRespList.end());
	}
	else
	{
		usleep(1000 * timeout);
	}
	return message_rsp_list.status;
}

int ZigbeeProtocol::OnDeviceAnnounce(uint8_t *buff, uint16_t len)
{
	LOGI("OnDeviceAnnounce");
	if (len != 11)
	{
		LOGW("DeviceAnnounce format error");
		return -1;
	}
	typedef struct
	{
		uint16_t nwkAddr;
		uint8_t ieeeAddr[8];
		uint8_t capability;
	} DeviceAnnounce_st;
	DeviceAnnounce_st *deviceAnnounce = (DeviceAnnounce_st *)buff;
	uint16_t devAddr = bswap_16(deviceAnnounce->nwkAddr);
	LOGI("OnDeviceAnnounce len: %d, nwkAddr: 0x%04X, capability: 0x%02X", len, devAddr, deviceAnnounce->capability);
	string mac = Util::ConvertU32ToHexString(deviceAnnounce->ieeeAddr, sizeof(deviceAnnounce->ieeeAddr));
	scanList[devAddr] = mac;
	// DiscoveryActiveEndpoint(devAddr);
	ReadAttribute(devAddr);
	return 0;
}

int ZigbeeProtocol::OnReportAttribute(uint8_t *buff, uint16_t len)
{
	LOGI("OnReportAttribute");
	if (len >= 8)
	{
		ZCLCmdRspHdr_st *zclCmdRspHdr = (ZCLCmdRspHdr_st *)buff;
		uint16_t srcAddr = bswap_16(zclCmdRspHdr->srcAddr);
		LOGD("srcAddr: 0x%04X, srcEp: %d, dstEp: %d, seqNum: %d", srcAddr, zclCmdRspHdr->srcEp, zclCmdRspHdr->dstEp, zclCmdRspHdr->seqNum);
		DeviceZigbee *deviceZigbee = gateway->getDeviceZigbeeFromAddr(srcAddr);
		if (deviceZigbee)
		{
			deviceZigbee->InputData(buff + 5, len - 5);
		}
		else
		{
			LOGW("Zigbee device 0x%04X not found", srcAddr);
		}
		return 0;
	}
	else
	{
		LOGW("OnReportAttribute format error");
	}
	return -1;
}

int ZigbeeProtocol::OnReadAttributeResp(uint8_t *buff, uint16_t len)
{
	LOGI("OnReadAttributeResp");
	typedef struct
	{
		uint16_t clusterID;
		uint8_t attrNum;
		uint8_t attrList[];
	} ReadAttributeResp_st;

	typedef struct
	{
		uint16_t attrID;
		uint8_t status;
		uint8_t dataType;
		uint8_t data[];
	} Attribute_st;

	uint16_t messageLen = len;
	int rs = 0;
	if (messageLen >= 8)
	{
		ZCLCmdRspHdr_st *zclCmdRspHdr = (ZCLCmdRspHdr_st *)buff;
		uint16_t srcAddr = bswap_16(zclCmdRspHdr->srcAddr);
		LOGD("srcAddr: 0x%04X, srcEp: %d, dstEp: %d, seqNum: %d", srcAddr, zclCmdRspHdr->srcEp, zclCmdRspHdr->dstEp, zclCmdRspHdr->seqNum)

		ReadAttributeResp_st *readAttributeResp = (ReadAttributeResp_st *)(buff + 5);
		uint16_t clusterID = bswap_16(readAttributeResp->clusterID);
		Attribute_st *attribute = (Attribute_st *)readAttributeResp->attrList;
		uint16_t attrID;
		uint8_t dataLen = 0;
		if (clusterID == CLUSTER_GENERAL_BASIC)
		{
			uint8_t zclVersion = 0;
			uint8_t appVersion = 0;
			string manufacturerName = "";
			string modelIdentifier = "";
			uint8_t powerSource = 0;
			messageLen -= 8;
			for (int i = 0; i < readAttributeResp->attrNum; i++)
			{
				if (messageLen < 5)
				{
					LOGW("Attribute message err");
					rs = 1;
					break;
				}
				dataLen = 0;
				attrID = bswap_16(attribute->attrID);
				LOGD("attrID: 0x%04X", attrID);
				if (attribute->status == ZIGBEE_SUCCESS)
				{
					if (attrID == ATTRIBUTE_BASIC_ZCLVersion)
					{
						if (attribute->dataType == ZIGBEE_DATATYPE_UINT8)
						{
							zclVersion = attribute->data[0];
							dataLen = 1;
						}
					}
					else if (attrID == ATTRIBUTE_BASIC_ApplicationVersion)
					{
						if (attribute->dataType == ZIGBEE_DATATYPE_UINT8)
						{
							appVersion = attribute->data[0];
							dataLen = 1;
						}
					}
					else if (attrID == ATTRIBUTE_BASIC_ManufacturerName)
					{
						if (attribute->dataType == ZIGBEE_DATATYPE_STRING)
						{
							for (int j = 1; j <= attribute->data[0]; j++)
							{
								manufacturerName += attribute->data[j];
							}
							dataLen = attribute->data[0] + 1;
						}
					}
					else if (attrID == ATTRIBUTE_BASIC_ModelIdentifier)
					{
						if (attribute->dataType == ZIGBEE_DATATYPE_STRING)
						{
							for (int j = 1; j <= attribute->data[0]; j++)
							{
								modelIdentifier += attribute->data[j];
							}
							dataLen = attribute->data[0] + 1;
						}
					}
					else if (attrID == ATTRIBUTE_BASIC_PowerSource)
					{
						if (attribute->dataType == ZIGBEE_DATATYPE_ENUM8)
						{
							powerSource = attribute->data[0];
							dataLen = 1;
						}
					}
					else
					{
						LOGW("Read Attribute response not handle attribute id: 0x%04X", attrID);
					}
				}
				else
				{
					LOGW("Read Attribute response status err: %d, id: 0x%04X", attribute->status, attrID);
				}
				attribute = (Attribute_st *)((uint8_t *)attribute + dataLen + 4);
				messageLen -= dataLen + 4;
			}
			LOGI("addr: 0x%04X, zclVersion: %d, appVersion: %d, manufacturerName: %s, modelIdentifier: %s, powerSource: 0x%02X", srcAddr, zclVersion, appVersion, manufacturerName.c_str(), modelIdentifier.c_str(), powerSource);
			uint32_t type = Device::ConvertModelToDeviceType(modelIdentifier);
			string mac = scanList[srcAddr];
			scanList.erase(srcAddr);
			LOGI("addr: 0x%04X, type: 0x%04X, mac: %s", srcAddr, type, mac.c_str());
			if (type && mac != "")
			{
				Device *device = gateway->getDevice(mac);
				if (device)
				{
					LOGI("Update device");
					device->SetAddr(srcAddr);
					database->DeviceUpdate(device);
				}
				else
				{
					device = gateway->AddNewDevice("Zigbee_" + mac, Device::ConvertDeviceTypeToName(type), mac, srcAddr, type, true, true);
				}
				if (device)
					gateway->AddDeviceToScanList(device);
			}
		}
	}
	else
	{
		LOGW("OnReadAttributeResp format error");
	}
	return rs;
}

int ZigbeeProtocol::CommissionFormation()
{
	LOGD("CommissionFormation");
	int rs = SendMessage(ZBHCI_CMD_BDB_COMMISSION_FORMATION, 0, 0, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == 0)
	{
		LOGD("CommissionFormation ok");
	}
	else
	{
		LOGE("Send CommissionFormation error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::ResetFactory()
{
	LOGD("ResetFactory");
	int rs = SendMessage(ZBHCI_CMD_BDB_FACTORY_RESET, 0, 0, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == 0)
	{
		LOGD("ResetFactory ok");
	}
	else
	{
		LOGE("Send ResetFactory error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::SetChannel(uint8_t channel)
{
	LOGD("SetChannel");
	int rs = SendMessage(ZBHCI_CMD_BDB_CHANNEL_SET, &channel, 1, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 5000);
	if (rs == 0)
	{
		LOGD("SetChannel ok");
	}
	else
	{
		LOGE("Send SetChannel error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::DiscoverySimpleDescription(uint16_t addr, uint8_t endpoint)
{
	LOGD("DiscoverySimpleDescription");
	typedef struct
	{
		uint16_t dstAddr;
		uint16_t nwkAddrOfInterest;
		uint8_t endpoint;
	} discovery_simple_description_t;
	discovery_simple_description_t discovery_simple_description;
	discovery_simple_description.dstAddr = bswap_16(addr);
	discovery_simple_description.nwkAddrOfInterest = bswap_16(addr);
	discovery_simple_description.endpoint = endpoint;

	int rs = SendMessage(ZBHCI_CMD_DISCOVERY_SIMPLE_DESC_REQ, (uint8_t *)&discovery_simple_description, 5, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == 0)
	{
		LOGD("DiscoverySimpleDescription ok");
	}
	else
	{
		LOGE("Send DiscoverySimpleDescription error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::DiscoveryActiveEndpoint(uint16_t addr)
{
	LOGD("DiscoveryActiveEndpoint");
	typedef struct
	{
		uint16_t dstAddr;
		uint16_t nwkAddrOfInterest;
	} discovery_active_endpoint_t;
	discovery_active_endpoint_t discovery_active_endpoint;
	discovery_active_endpoint.dstAddr = bswap_16(addr);
	discovery_active_endpoint.nwkAddrOfInterest = bswap_16(addr);

	int rs = SendMessage(ZBHCI_CMD_DISCOVERY_ACTIVE_EP_REQ, (uint8_t *)&discovery_active_endpoint, 4, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == 0)
	{
		LOGD("DiscoveryActiveEndpoint ok");
	}
	else
	{
		LOGE("Send DiscoveryActiveEndpoint error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::PermitJoin(uint8_t duration)
{
	LOGD("PermitJoin");
	typedef struct
	{
		uint16_t dstAddr;
		uint8_t permitDuration;
		uint8_t TC_significance;
	} permit_join_req_t;
	permit_join_req_t permit_join_req = {
			.dstAddr = 0xFFFF,
			.permitDuration = duration,
			.TC_significance = 1};
	int rs = SendMessage(ZBHCI_CMD_MGMT_PERMIT_JOIN_REQ, (uint8_t *)&permit_join_req, sizeof(permit_join_req), ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == 0)
	{
		LOGD("PermitJoin ok");
	}
	else
	{
		LOGE("Send PermitJoin error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::ReadAttribute(uint16_t addr)
{
	LOGD("ReadAttribute");
	typedef struct
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint8_t dstAddr[2];
		uint8_t srcEp;
		uint8_t dstEp;

		uint8_t profileID[2];
		uint8_t direction;
		uint8_t clusterID[2];
		uint8_t attrNum;
		uint16_t attrList[32];
	} read_attribute_req_t;
	read_attribute_req_t read_attribute_req;
	read_attribute_req.dstAddrMode = 2;
	read_attribute_req.dstAddr[0] = (uint8_t)((addr >> 8) & 0xFF);
	read_attribute_req.dstAddr[1] = (uint8_t)(addr & 0xFF);
	read_attribute_req.srcEp = 0x01;
	read_attribute_req.dstEp = 0xFF;
	read_attribute_req.profileID[0] = (uint8_t)((PROFILE_ZHA >> 8) & 0xFF);
	read_attribute_req.profileID[1] = (uint8_t)(PROFILE_ZHA & 0xFF);
	read_attribute_req.direction = 0;
	read_attribute_req.clusterID[0] = (uint8_t)(CLUSTER_GENERAL_BASIC & 0xFF);
	read_attribute_req.clusterID[1] = (uint8_t)((CLUSTER_GENERAL_BASIC >> 8) & 0xFF);
	read_attribute_req.attrNum = 5;
	read_attribute_req.attrList[0] = ATTRIBUTE_BASIC_ZCLVersion;
	read_attribute_req.attrList[1] = ATTRIBUTE_BASIC_ApplicationVersion;
	read_attribute_req.attrList[2] = ATTRIBUTE_BASIC_ManufacturerName;
	read_attribute_req.attrList[3] = ATTRIBUTE_BASIC_ModelIdentifier;
	read_attribute_req.attrList[4] = ATTRIBUTE_BASIC_PowerSource;

	int rs = SendMessage(ZBHCI_CMD_ZCL_ATTR_READ, (uint8_t *)&read_attribute_req, 11 + 2 * read_attribute_req.attrNum, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == 0)
	{
		LOGD("ReadAttribute ok");
	}
	else
	{
		LOGE("Send ReadAttribute error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::AddGroup(uint16_t groupId, uint16_t devAddr, uint8_t epId)
{
	LOGD("AddGroup");
	typedef struct
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint8_t dstAddr[2];
		uint8_t srcEp;
		uint8_t dstEp;

		uint8_t groupId[2];
		uint8_t groupName[2];
	} add_group_t;
	add_group_t add_group;

	add_group.dstAddrMode = 2;
	add_group.dstAddr[0] = (uint8_t)((devAddr >> 8) & 0xFF);
	add_group.dstAddr[1] = (uint8_t)(devAddr & 0xFF);
	add_group.srcEp = 0x01;
	add_group.dstEp = epId;

	add_group.groupId[0] = (uint8_t)((groupId >> 8) & 0xFF);
	add_group.groupId[1] = (uint8_t)(groupId & 0xFF);;
	add_group.groupName[0] = 'a';
	add_group.groupName[1] = 'b';

	int rs = SendMessage(ZBHCI_CMD_ZCL_GROUP_ADD, (uint8_t *)&add_group, 9, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == 0)
	{
		LOGD("AddGroup ok");
	}
	else
	{
		LOGE("Send AddGroup error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::ZCLOnoffDevice(uint16_t devAddr, uint8_t func)
{
	LOGD("ZCLOnoffDevice");
	if (func > 2)
	{
		LOGW("ZCLOnoffDevice func not match: %d", func);
	}
	typedef struct
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint8_t dstAddr[2];
		uint8_t srcEp;
		uint8_t dstEp;
	} zcl_onoff_t;
	zcl_onoff_t zcl_onoff;
	zcl_onoff.dstAddrMode = 2;
	zcl_onoff.dstAddr[0] = (uint8_t)((devAddr >> 8) & 0xFF);
	zcl_onoff.dstAddr[1] = (uint8_t)(devAddr & 0xFF);
	zcl_onoff.srcEp = 0x01;
	zcl_onoff.dstEp = 0xFF;

	int rs = SendMessage(ZBHCI_CMD_ZCL_ONOFF_ON + func, (uint8_t *)&zcl_onoff, 5, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == 0)
	{
		LOGD("ZCLOnoffDevice ok");
	}
	else
	{
		LOGE("Send ZCLOnoffDevice error, rs: %d", rs);
	}
	return rs;
}

int ZigbeeProtocol::ZCLOnoffGroup(uint16_t groupAddr, uint8_t func)
{
	LOGD("ZCLOnoffGroup");
	if (func > 2)
	{
		LOGW("ZCLOnoffGroup func not match: %d", func);
	}
	typedef struct
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint8_t dstAddr[2];
		uint8_t srcEp;
	} zcl_onoff_t;
	zcl_onoff_t zcl_onoff;
	zcl_onoff.dstAddrMode = 1;
	zcl_onoff.dstAddr[0] = (uint8_t)((groupAddr >> 8) & 0xFF);
	zcl_onoff.dstAddr[1] = (uint8_t)(groupAddr & 0xFF);
	zcl_onoff.srcEp = 0x01;

	int rs = SendMessage(ZBHCI_CMD_ZCL_ONOFF_ON + func, (uint8_t *)&zcl_onoff, 4, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == 0)
	{
		LOGD("ZCLOnoffGroup ok");
	}
	else
	{
		LOGE("Send ZCLOnoffGroup error, rs: %d", rs);
	}
	return rs;
}

#endif
