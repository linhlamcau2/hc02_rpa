#pragma once
#include <string>
#include <string.h>
#include <sqlite3.h>
#include <vector>
#include <mutex>
#include <Device.h>
#include <Group.h>
#include <Gateway.h>
#include "SceneBle.h"
#include "Room.h"

#ifdef ESP_PLATFORM
#define DB_NAME "/spiffs/smh.sqlite"
#elif defined(__ANDROID__)
#define DB_NAME "/etc/smh/smh.sqlite"
#else
#define DB_NAME "/smh.sqlite"
#endif

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

	int DeviceRead();
	int DeviceAdd(Device *device);
	int DeviceUpdate(Device *device);
	int DeviceDel(Device *device);
	int DeviceDel(string mac);
	int DeviceDelAll();

	int GatewayRead();
	int GatewayAdd(Gateway *gateway);
	int GatewayUpdate(Gateway *gateway);
	int GatewayUpdateId(Gateway *gateway, string id);
	int GatewayUpdateNetKey(Gateway *gateway, string netkey);
	int GatewayUpdateAppKey(Gateway *gateway, string appkey);
	int GatewayUpdateDeviceKey(Gateway *gateway, string devicekey);
	int GatewayUpdateUnicast(Gateway *gateway, uint16_t unicast);
	int GatewayUpdateDormitory(Gateway *gateway, string dormitory);
	int GatewayUpdateRefreshToken(Gateway *gateway, string refreshToken);
	int GatewayDel(Gateway *gateway);
	int GatewayDel(string id);
	int GatewayDelAll();

	int DeviceAttributeRead();
	int DeviceAttributeAdd(Device *device, int attributeId, double value);
	int DeviceAttributeUpdate(Device *device, int attributeId, double value);
	int DeviceAttributeAddOrReplace(Device *device, int attributeId, double value);
	int DeviceAttributeDel(Device *device, int attributeId);
	int DeviceAttributeDelAll();

	int GroupRead();
	int GroupAdd(Group *group);
	int GroupUpdate(Group *group);
	int GroupDel(Group *group);
	int GroupDel(string id);
	int GroupDelAll();

	int DeviceInGroupRead();
	int DeviceInGroupAdd(Group *group, Device *device, int epId);
	int DeviceInGroupDel(Group *group, Device *device, int epId);
	int DeviceInGroupDelAll();

	// int DeviceSceneRead();
	// int DeviceSceneAdd(string mac, string schedule);
	// int DeviceSceneAdd(Device *device, string schedule);
	// int DeviceSceneDel(string mac, string schedule);
	// int DeviceSceneDel(Device *device, string schedule);

	int RuleRead();
	int RuleAdd(string id, string rule, int inEnable, int type);
	int RuleUpdate(string id, string rule);
	int RuleUpdateStatus(string id, int isEnable);
	int RuleDel(string id);

	int SceneBleRead();
	int DeviceInSceneBleAdd(SceneBle *scene, Device *device, Json::Value data);
	int DeviceInSceneBleDel(SceneBle *scene, Device *device, int epId);
	int SceneBleDel(SceneBle *scene);

    	int RoomRead();
	int RoomAdd(Room *room);
	int RoomUpdate(Room *room, int id);
	int RoomDel(Room *room);
	int RoomDelAll();

	int DeviceInRoomRead();
	int DeviceInRoomAdd(Room *room, Device *device);
	int DeviceInRoomDel(Room *room, Device *device);
	int DeviceInRoomDel(Room *room);

	int DataRoomRead();
	int DataRoomAdd(string id, string data);
	int DataRoomUpdate(string id, string data);
	int DataRoomDel(string id, string data);
	int DataRoomDelAll();

	int DeviceChildRead();
	int DeviceChildAdd(string deviceId, int element);
	int DeviceChildUpdate(string deviceId, int element);
	int DeviceChildDel(string deviceId);
	int DeviceChildDelAll();
};

extern Db *database;
