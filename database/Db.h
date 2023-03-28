#pragma once
#include <string>
#include <string.h>
#include <sqlite3.h>
#include <vector>
#include <mutex>
#include "Device.h"
#include "Group.h"
#include "Gateway.h"
#include "SceneBle.h"
#include "Room.h"

using namespace std;

class Db
{

private:
	pthread_mutex_t mutex;

	int Sqlite_Exec(string &sql);
	int ReadAll(string table, void *listPtr, int (*Parse)(sqlite3_stmt *, void *));

public:
	Db();
	~Db() {}

	void init(void);
	int createTableIfNotExists();

	int DeviceRead();
	int DeviceAdd(Device *device);
	int DeviceUpdate(Device *device);
	int DeviceDel(Device *device);
	int DeviceDel(string mac);
	int DeviceDelAll();

	int DeviceAttributeRead();
	int DeviceAttributeAdd(Device *device, int attributeId, double value);
	int DeviceAttributeUpdate(Device *device, int attributeId, double value);
	int DeviceAttributeAddOrReplace(Device *device, int attributeId, double value);
	int DeviceAttributeDel(Device *device, int attributeId);
	int DeviceAttributeDelAll();

	int DeviceBleChildRead();
	int DeviceBleChildAdd(string deviceId, int element);
	int DeviceBleChildUpdate(string deviceId, int element);
	int DeviceBleChildDel(string deviceId);
	int DeviceBleChildDelAll();

	int DeviceInGroupRead();
	int DeviceInGroupAdd(Group *group, Device *device, int epId);
	int DeviceInGroupDel(Group *group, Device *device, int epId);
	int DeviceInGroupDelAll();

	int DeviceInRoomRead();
	int DeviceInRoomAdd(Room *room, Device *device);
	int DeviceInRoomDel(Room *room, Device *device);
	int DeviceInRoomDelAll();

	int DeviceInSceneBleRead();
	int DeviceInSceneBleAdd(SceneBle *scene, Device *device, string data);
	int DeviceInSceneBleDel(SceneBle *scene, Device *device);
	int DeviceInSceneBleDelAll();

	int GatewayRead();
	int GatewayAdd(Gateway *gateway);
	int GatewayUpdateId(Gateway *gateway, string id);
	int GatewayUpdateNetKey(Gateway *gateway, string netkey);
	int GatewayUpdateAppKey(Gateway *gateway, string appkey);
	int GatewayUpdateDeviceKey(Gateway *gateway, string devicekey);
	int GatewayUpdateUnicast(Gateway *gateway, uint16_t unicast);
	int GatewayUpdateIvIndex(Gateway *gateway, uint32_t iv_index);
	int GatewayUpdateDormitory(Gateway *gateway, string dormitory);
	int GatewayUpdateRefreshToken(Gateway *gateway, string refreshToken);
	int GatewayDel(Gateway *gateway);
	int GatewayDel(string id);
	int GatewayDelAll();

	int GroupRead();
	int GroupAdd(Group *group);
	int GroupUpdate(Group *group);
	int GroupDel(Group *group);
	int GroupDel(string id);
	int GroupDelAll();

	int RoomRead();
	int RoomAdd(Room *room);
	int RoomUpdate(Room *room, int id);
	int RoomDel(Room *room);
	int RoomDelAll();

	int RuleRead();
	int RuleAdd(Rule *rule, string data, int type, bool enable);
	int RuleUpdateData(Rule *rule, string data);
	int RuleUpdateStatus(Rule *rule, bool enable);
	int RuleUpdateType(Rule *rule, int type);
	int RuleDel(Rule *rule);
	int RuleDelAll();

	int SceneBleRead();
	int SceneBleAdd(SceneBle *scene);
	int SceneBleUpdate(SceneBle *scene);
	int SceneBleDel(SceneBle *scene);
	int SceneBleDelAll();
};

extern Db *database;
