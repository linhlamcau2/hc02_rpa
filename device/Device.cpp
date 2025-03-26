#include "Device.h"
#include "Gateway.h"
#include "Log.h"
#include <functional>
#include <unistd.h>
#include <Util.h>

Device::Device(string id, string name, string mac, Json::Value &dataJson, uint16_t addr, uint32_t type, uint16_t version) : Object(id, addr, name)
{
	this->mac = mac;
	this->type = type;
	this->dataJson = dataJson;
	this->version = version;
	this->isFavorite = false;
	propertyJsonUpdate = Json::objectValue;

	lastOnlineState = false;
	lastTimeActive = 0;
	lastTimeCheckActive = 0;
}

Device::~Device()
{
}

string Device::GetMac()
{
	return mac;
}

Json::Value Device::GetData()
{
	return dataJson;
}

string Device::GetDeviceKey()
{
	return "";
}

uint32_t Device::GetType()
{
	return type;
}

uint16_t Device::GetVersion()
{
	return version;
}

string Device::GetVersionStr()
{
	return to_string((version >> 8) & 0xFF) + "." + to_string(version & 0xFF);
}

int Device::GetRSSI()
{
	return rssi;
}

void Device::SetRSSI(int rssi)
{
	this->rssi = rssi;
}

bool Device::isOnline()
{
	return lastOnlineState;
}


int Device::BuildAttributesValue(Json::Value &pushDataValue)
{
	Json::Value deviceData;
	deviceData["gateway"] = "Farm Gateway RAL";
	deviceData["name"] = name;
	deviceData["mac"] = mac;
	deviceData["type"] = (int)type;
	pushDataValue[id] = deviceData;
	return CODE_OK;
}

void Device::DeviceInputData(uint8_t *data, int len, uint16_t addr)
{
	InputData(data, len, addr);
}

void Device::UpdateLastTimeActive()
{
	lastTimeActive = time(NULL);
}

int Device::DoJsonArray(Json::Value &dataValue)
{
	if (dataValue.isArray())
	{
		for (Json::ArrayIndex i = 0; i < dataValue.size(); i++)
		{
			Do(dataValue[i]);
		}
	}
	else
	{
		Do(dataValue);
	}
	return CODE_OK;
}

void Device::SetPropertyJsonUpdate(Json::Value property)
{
	if (this->propertyJsonUpdate != property)
	{
		PushTelemetry(property);
		this->propertyJsonUpdate = property;
	}
}

void Device::UpdatePropertyJsonUpdate(Json::Value &propertyUpdate)
{
	if (propertyUpdate.isObject() && this->propertyJsonUpdate.isObject())
	{
		for (auto const &key : propertyUpdate.getMemberNames())
		{
			if (this->propertyJsonUpdate.isMember(key))
			{
				if (propertyUpdate[key].type() == this->propertyJsonUpdate[key].type())
				{
					this->propertyJsonUpdate[key] = propertyUpdate[key];
				}
			}
		}
	}
}

Json::Value Device::GetPropertyJsonUpdate()
{
	return this->propertyJsonUpdate;
}

int Device::PushTelemetry()
{
	Json::Value pushData;
	int build = BuildTelemetryValue(pushData);
	if (build == 0)
	{
		return gateway->CloudPublish(pushData);
	}
	return CODE_ERROR;
}

int Device::PushTelemetry(Json::Value &jsonValue)
{
	if (!jsonValue.isNull())
	{
		Json::Value deviceData;
		Json::Value devicesData;
		Json::Value dataValue;
		deviceData["id"] = id;
		deviceData["data"] = jsonValue;
		devicesData.append(deviceData);
		dataValue["device"] = devicesData;
		gateway->pushDeviceUpdateLocal(dataValue);
		gateway->pushDeviceUpdateCloud(dataValue);
		return CODE_OK;
	}
	return CODE_ERROR;
}

int Device::PushAttributes()
{
	LOGD("PushAttributes");
	Json::Value pushData;
	int build = BuildAttributesValue(pushData);
	if (build == 0)
	{
		return gateway->CloudPublish(pushData);
	}
	return CODE_ERROR;
}

int Device::PushAttributes(Json::Value &jsonValue)
{
	if (!jsonValue.isNull())
		return gateway->CloudPublish(jsonValue);
	return CODE_ERROR;
}

// TODO: remove
static map<uint32_t, const char *> typeToNameList;
// static map<string, uint32_t> modelToTypeList;
static map<uint32_t, uint32_t> bleTypeToGroupIdList;
static map<uint16_t, const char *> bleAttributeIdToAttributeString;

// TODO: Check list device to Name
void Device::InitDeviceModelList()
{
}

void Device::RegisterDeviceModel(uint32_t type, string model, const char *name)
{
	typeToNameList[type] = name;
}

uint32_t Device::BleTypeToGroupId(uint32_t deviceType)
{
	return bleTypeToGroupIdList[deviceType];
}

const char *Device::BleAttributeIdToAttributeStr(uint16_t attributeId)
{
	return bleAttributeIdToAttributeString[attributeId];
}

const char *Device::ConvertDeviceTypeToName(uint32_t type)
{
	return typeToNameList[type];
}

uint32_t Device::ConverPidToDeviveType(uint16_t pid)
{
	uint8_t type1 = (pid >> 12) & 0x0F;
	uint8_t type2 = (pid >> 8) & 0x0F;
	uint8_t type3 = (pid) & 0xFF;
	return (type3 + (type2 * 1000) + (type1 * 10000));
}

bool Device::GetIsFavorite()
{
	return this->isFavorite;
}

bool Device::SetIsFavorite(bool isFavorite)
{
	this->isFavorite = isFavorite;
	return this->isFavorite;
}
