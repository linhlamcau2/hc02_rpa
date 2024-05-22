#ifdef CONFIG_ENABLE_ZIGBEE

#include "ZigbeeProtocol.h"
#include <stdlib.h>
#include <thread>
#include <functional>
#include <byteswap.h>
#include "Log.h"
#include <Util.h>
#include <string.h>
#include <algorithm>
#include <Db.h>
#include "DeviceZigbee.h"

#include "cluster/basic/ClusterBasic.h"

ZigbeeProtocol *zigbeeProtocol = NULL;

ZigbeeProtocol::ZigbeeProtocol(char *uartPort, int baudrate) : Uart(uartPort, baudrate, 100000)
{
}

ZigbeeProtocol::~ZigbeeProtocol()
{
}

void ZigbeeProtocol::HandleOpcodeBleThread()
{
	LOGI("Start HandleOpcodeBleThread");
	message_rsp_st *message_rsp = NULL;
	while (1)
	{
		if (GetOpcodeExceptionMessage(&message_rsp) == CODE_OK)
		{
			CheckOpcodeException(message_rsp);
			free(message_rsp);
		}
		usleep(100000);
	}
}

void ZigbeeProtocol::init()
{
	Uart::init();
	usleep(100000);

	RegisterCmdCallback(ZBHCI_CMD_NODES_DEV_ANNCE_IND, bind(&ZigbeeProtocol::OnDeviceAnnounce, this, placeholders::_1, placeholders::_2));
	RegisterCmdCallback(ZBHCI_CMD_ZCL_REPORT_MSG_RCV, bind(&ZigbeeProtocol::OnReportAttribute, this, placeholders::_1, placeholders::_2));
	RegisterCmdCallback(ZBHCI_CMD_ZCL_ATTR_READ_RSP, bind(&ZigbeeProtocol::OnReadAttributeResp, this, placeholders::_1, placeholders::_2));

	thread handleOpcodeBleThread(bind(&ZigbeeProtocol::HandleOpcodeBleThread, this));
	handleOpcodeBleThread.detach();
}

int ZigbeeProtocol::RegisterCmdCallback(uint16_t type, OnCmdCallbackFunc onCmdCallbackFunc)
{
	LOGD("RegisterCmd type: 0x%04X", type);
	onCmdCallbackFuncList[type] = onCmdCallbackFunc;
	return CODE_OK;
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

int ZigbeeProtocol::GetOpcodeExceptionMessage(message_rsp_st **data)
{
	int rs = CODE_ERROR;
	vectorCheckOpcodeMtx.lock();
	if (messageCheckOpcodeList.size() > 0)
	{
		*data = messageCheckOpcodeList[0];
		messageCheckOpcodeList.erase(messageCheckOpcodeList.begin());
		rs = CODE_OK;
	}
	vectorCheckOpcodeMtx.unlock();
	return rs;
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
		if (rs == CODE_OK)
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

int ZigbeeProtocol::OnMessage(unsigned char *data, int len)
{
	LOGD("OnMessage len: %d", len);
	uint8_t *message = data;
	int lenRemain = len;
	message_rsp_st *message_rsp = (message_rsp_st *)message;
	Util::LedZigbee(false);
	Util::LedServiceLock();
	while (lenRemain >= sizeof(message_rsp_st) && message_rsp->header == MESSAGE_HEADER)
	{
		uint16_t type = bswap_16(message_rsp->type);
		uint16_t payloadLen = bswap_16(message_rsp->len);
		uint16_t packageLen = payloadLen + sizeof(message_rsp_st) + 1;
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
				vectorCheckOpcodeMtx.lock();
				if (messageCheckOpcodeList.size() < ZIGBEE_CHECK_OPCODE_BUFFER_MAX_SIZE)
				{
					message_rsp_st *messageCheckOpcode = (message_rsp_st *)malloc(packageLen);
					memcpy(messageCheckOpcode, message_rsp, packageLen);
					messageCheckOpcodeList.push_back(messageCheckOpcode);
				}
				vectorCheckOpcodeMtx.unlock();
			}
		}
		lenRemain -= packageLen;
		message += packageLen;
		message_rsp = (message_rsp_st *)message;
	}
	Util::LedZigbee(true);
	Util::LedServiceUnlock();
	return lenRemain;
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

	message_req_st *message_req = (message_req_st *)buff;
	message_req->header = MESSAGE_HEADER;
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

		while (message_rsp_list.status == 0xFF && timeout--)
		{
			usleep(1000);
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
		return CODE_ERROR;
	}
	typedef struct __attribute__((packed))
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
	return CODE_OK;
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
			int clusterLen = len - sizeof(ZCLCmdRspHdr_st);
			typedef struct __attribute__((packed))
			{
				uint16_t clusterId;
				uint8_t attrNum;
				uint8_t data[];
			} ClusterMessage_st;
			ClusterMessage_st *clusterMessage = (ClusterMessage_st *)zclCmdRspHdr->data;

			int attrLen = clusterLen - sizeof(ClusterMessage_st);
			typedef struct __attribute__((packed))
			{
				uint16_t attrID;
				uint8_t dataType;
				uint8_t data[];
			} AttrMessage_st;
			AttrMessage_st *attrMessage = NULL;

			if (clusterLen > sizeof(ClusterMessage_st))
			{
				LOGD("clusterMessage->clusterId: 0x%04X, clusterMessage->attrNum: %d", bswap_16(clusterMessage->clusterId), clusterMessage->attrNum);
				uint8_t *attrData = clusterMessage->data;
				if (bswap_16(clusterMessage->clusterId) == 0x0000)
				{
					string model;
					uint8_t appVersion = 0, zclVersion = 0;
					for (int i = 0; i < clusterMessage->attrNum; i++)
					{
						attrMessage = (AttrMessage_st *)attrData;
						LOGD("Attribute ID: 0x%04X", bswap_16(attrMessage->attrID));
						if (bswap_16(attrMessage->attrID) == 0x0001)
						{
							if (attrMessage->dataType == ZCL_DATA_TYPE_UINT8)
							{
								appVersion = attrMessage->data[0];
								LOGW("appVersion: %d", appVersion);
							}
						}
						else if (bswap_16(attrMessage->attrID) == 0x0005)
						{
							if (attrMessage->dataType == ZCL_DATA_TYPE_CHAR_STR)
							{
								for (int i = 0; i < attrMessage->data[0]; i++)
								{
									model += attrMessage->data[i + 1];
								}
								LOGW("model: %s", model.c_str());
							}
						}
						int dataSize = getSizeOfDataType(&attrMessage->dataType);
						attrData += 3 + dataSize;
						attrLen -= 3 + dataSize;
					}

					if (!model.empty())
					{
						uint32_t type = Device::ConvertModelToDeviceType(model);
						string mac = scanList[srcAddr];
						scanList.erase(srcAddr);
						LOGI("addr: 0x%04X, type: 0x%04X, mac: %s", srcAddr, type, mac.c_str());
						if (type && mac != "")
						{
							Json::Value dataJson;
							Device *device = gateway->AddNewDevice(Util::GenUuidFromMac(mac), Util::setString(Device::ConvertDeviceTypeToName(type)), mac, dataJson, srcAddr, type, zclVersion | appVersion << 8, true);
							if (device)
								gateway->AddDeviceToScanList(device);
						}
					}
					return CODE_OK;
				}
			}
		}
		return CODE_OK;
	}
	else
	{
		LOGW("OnReportAttribute format error");
	}
	return CODE_ERROR;
}

int ZigbeeProtocol::OnReadAttributeResp(uint8_t *buff, uint16_t len)
{
	LOGI("OnReadAttributeResp");
	typedef struct __attribute__((packed))
	{
		uint16_t clusterID;
		uint8_t attrNum;
		uint8_t attrList[];
	} ReadAttributeResp_st;

	typedef struct __attribute__((packed))
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

		ReadAttributeResp_st *readAttributeResp = (ReadAttributeResp_st *)zclCmdRspHdr->data;
		uint16_t clusterID = bswap_16(readAttributeResp->clusterID);
		uint8_t attrNum = readAttributeResp->attrNum;
		LOGD("clusterID: 0x%04X, attrNum: %d", clusterID, attrNum);
		Attribute_st *attribute = (Attribute_st *)readAttributeResp->attrList;
		uint16_t attrID;
		int dataLen = 0;
		if (clusterID == ZCL_CLUSTER_GEN_BASIC)
		{
			uint8_t zclVersion = 0;
			uint8_t appVersion = 0;
			string manufacturerName = "";
			string modelIdentifier = "";
			uint8_t powerSource = 0;
			messageLen -= 8;
			for (int i = 0; i < attrNum; i++)
			{
				if (messageLen < 5)
				{
					LOGW("Attribute message err");
					rs = 1;
					break;
				}
				attrID = bswap_16(attribute->attrID);
				LOGD("attrID: 0x%04X", attrID);
				if (attribute->status == ZIGBEE_SUCCESS)
				{
					if (attrID == ATTRIBUTE_BASIC_ZCLVersion)
					{
						if (attribute->dataType == ZCL_DATA_TYPE_UINT8)
						{
							zclVersion = attribute->data[0];
						}
					}
					else if (attrID == ATTRIBUTE_BASIC_ApplicationVersion)
					{
						if (attribute->dataType == ZCL_DATA_TYPE_UINT8)
						{
							appVersion = attribute->data[0];
						}
					}
					else if (attrID == ATTRIBUTE_BASIC_ManufacturerName)
					{
						if (attribute->dataType == ZCL_DATA_TYPE_CHAR_STR)
						{
							for (int j = 1; j <= attribute->data[0]; j++)
							{
								manufacturerName += attribute->data[j];
							}
						}
					}
					else if (attrID == ATTRIBUTE_BASIC_ModelIdentifier)
					{
						if (attribute->dataType == ZCL_DATA_TYPE_CHAR_STR)
						{
							for (int j = 1; j <= attribute->data[0]; j++)
							{
								modelIdentifier += attribute->data[j];
							}
						}
					}
					else if (attrID == ATTRIBUTE_BASIC_PowerSource)
					{
						if (attribute->dataType == ZCL_DATA_TYPE_ENUM8)
						{
							powerSource = attribute->data[0];
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
				dataLen = getSizeOfDataType(&attribute->dataType);
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
				Device *device = gateway->getDeviceFromMac(mac);
				if (device)
				{
					LOGI("Update device");
					device->SetAddr(srcAddr);
					database->DeviceUpdate(device);
				}
				else
				{
					Json::Value dataJson;
					device = gateway->AddNewDevice(Util::GenUuidFromMac(mac), Util::setString(Device::ConvertDeviceTypeToName(type)), mac, dataJson, srcAddr, type, zclVersion | appVersion << 8, true);
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
	if (rs == CODE_OK)
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
	if (rs == CODE_OK)
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
	if (rs == CODE_OK)
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
	typedef struct __attribute__((packed))
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
	if (rs == CODE_OK)
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
	typedef struct __attribute__((packed))
	{
		uint16_t dstAddr;
		uint16_t nwkAddrOfInterest;
	} discovery_active_endpoint_t;
	discovery_active_endpoint_t discovery_active_endpoint;
	discovery_active_endpoint.dstAddr = bswap_16(addr);
	discovery_active_endpoint.nwkAddrOfInterest = bswap_16(addr);

	int rs = SendMessage(ZBHCI_CMD_DISCOVERY_ACTIVE_EP_REQ, (uint8_t *)&discovery_active_endpoint, 4, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
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
	typedef struct __attribute__((packed))
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
	if (rs == CODE_OK)
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
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
		uint8_t dstEp;
		uint16_t profileID;
		uint8_t direction;
		uint16_t clusterID;
		uint8_t attrNum;
		uint16_t attrList[32];
	} read_attribute_req_t;
	read_attribute_req_t read_attribute_req;
	read_attribute_req.dstAddrMode = 2;
	read_attribute_req.dstAddr = bswap_16(addr);
	read_attribute_req.srcEp = 0x01;
	read_attribute_req.dstEp = 0xFF;
	read_attribute_req.profileID = bswap_16(PROFILE_ZHA);
	read_attribute_req.direction = 0;
	read_attribute_req.clusterID = bswap_16(ZCL_CLUSTER_GEN_BASIC);
	read_attribute_req.attrNum = 5;
	read_attribute_req.attrList[0] = bswap_16(ATTRIBUTE_BASIC_ZCLVersion);
	read_attribute_req.attrList[1] = bswap_16(ATTRIBUTE_BASIC_ApplicationVersion);
	read_attribute_req.attrList[2] = bswap_16(ATTRIBUTE_BASIC_ManufacturerName);
	read_attribute_req.attrList[3] = bswap_16(ATTRIBUTE_BASIC_ModelIdentifier);
	read_attribute_req.attrList[4] = bswap_16(ATTRIBUTE_BASIC_PowerSource);

	int rs = SendMessage(ZBHCI_CMD_ZCL_ATTR_READ, (uint8_t *)&read_attribute_req, 11 + 2 * read_attribute_req.attrNum, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
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
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
		uint8_t dstEp;
		uint16_t groupId;
		uint16_t groupName;
	} add_group_t;
	add_group_t add_group;

	add_group.dstAddrMode = 2;
	add_group.dstAddr = bswap_16(devAddr);
	add_group.srcEp = 0x01;
	add_group.dstEp = epId;
	add_group.groupId = bswap_16(groupId);
	add_group.groupName = bswap_16(0x4142);

	int rs = SendMessage(ZBHCI_CMD_ZCL_GROUP_ADD, (uint8_t *)&add_group, 9, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
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
	uint16_t codeFunc;
	if (func == 0)
		codeFunc = ZBHCI_CMD_ZCL_ONOFF_OFF;
	else if (func == 1)
		codeFunc = ZBHCI_CMD_ZCL_ONOFF_ON;
	else if (func == 2)
		codeFunc = ZBHCI_CMD_ZCL_ONOFF_TOGGLE;
	else
	{
		return CODE_ERROR;
		LOGW("ZCLOnoffDevice func not match: %d", func);
	}
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
		uint8_t dstEp;
	} zcl_onoff_t;
	zcl_onoff_t zcl_onoff;
	zcl_onoff.dstAddrMode = 2;
	zcl_onoff.dstAddr = bswap_16(devAddr);
	zcl_onoff.srcEp = 0x01;
	zcl_onoff.dstEp = 0xFF;

	int rs = SendMessage(codeFunc, (uint8_t *)&zcl_onoff, 5, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
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
	typedef struct __attribute__((packed))
	{
		// ZCLCmdHdr
		uint8_t dstAddrMode;
		uint16_t dstAddr;
		uint8_t srcEp;
	} zcl_onoff_t;
	zcl_onoff_t zcl_onoff;
	zcl_onoff.dstAddrMode = 1;
	zcl_onoff.dstAddr = bswap_16(groupAddr);
	zcl_onoff.srcEp = 0x01;

	int rs = SendMessage(ZBHCI_CMD_ZCL_ONOFF_ON + func, (uint8_t *)&zcl_onoff, 4, ZBHCI_CMD_ACKNOWLEDGE, 0, 0, 2000);
	if (rs == CODE_OK)
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
