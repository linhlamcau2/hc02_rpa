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
#ifdef __ANDROID__
#include "Noti.h"
#endif

using namespace std;

class Db
{

private:
	sqlite3 *db;
	mutex mtx;

	int Sqlite_Exec(string &sql);
	int ReadAll(string table, void *listPtr, int (*Parse)(sqlite3_stmt *, void *));

public:
	Db();
	~Db();

	void init(void);
	bool IsHaveDb();
	int createTableIfNotExists();

	int Sqlite_BenginTransaction();
	int Sqlite_EndTransaction();

	int DeviceRead();
	int DeviceAdd(Device *device);
	int DeviceUpdate(Device *device);
	int DeviceUpdateData(Device *device);
	int DeviceDel(Device *device);
	int DeviceDel(string mac);
	int DeviceDelAll();
	int DelDevExist(Device *device);
	int DeviceUpdateFavorite(Device *device);

	int DeviceAttributeRead();
	int DeviceAttributeAdd(Device *device, string attribute, double value);
	int DeviceAttributeUpdate(Device *device, string attribute, double value);
	int DeviceAttributeAddOrReplace(Device *device, string attribute, double value);
	int DeviceAttributeDel(Device *device, string attribute);
	int DeviceAttributeDelAll();

	int DeviceBleChildRead();
	int DeviceBleChildAdd(Device *child, Device *parent, string data);
	int DeviceBleChildUpdateData(Device *child, Device *parent, string data);
	int DeviceBleChildDel(Device *child, Device *parent);
	int DeviceBleChildDelAll();

	int DeviceInGroupRead();
	int DeviceInGroupAdd(Group *group, Device *device, int epId);
	int DeviceInGroupDel(Group *group, Device *device, int epId);
	int DeviceInGroupDelDev(string deviceId);
	int DeviceInGroupDelDev(Device *device);
	int DeviceInGroupDelAll();

	int DeviceInRoomRead();
	int DeviceInRoomAdd(Room *room, Device *device);
	int DeviceInRoomDel(Room *room, Device *device);
	int DeviceInRoomDelDev(string deviceId);
	int DeviceInRoomDelDev(Device *device);
	int DeviceInRoomDelAll();

	int DeviceInSceneBleRead();
	int DeviceInSceneBleAdd(SceneBle *sceneBle, Device *device, string data);
	int DeviceInSceneBleDel(SceneBle *sceneBle, Device *device);
	int DeviceInSceneBleDelDev(string deviceId);
	int DeviceInSceneBleDelDev(Device *device);
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
	int GatewayUpdateData(Gateway *gateway, string data);
	int GatewayUpdateVersion(Gateway *gateway, string version);
	int GatewayDel(Gateway *gateway);
	int GatewayDel(string id);
	int GatewayDelAll();

	int GroupRead();
	int GroupAdd(Group *group);
	int GroupUpdate(Group *group);
	int GroupUpdateRoom(Group *group, string roomId);
	int GroupDel(Group *group);
	int GroupDel(string id);
	int GroupDelAll();

	int RoomRead();
	int RoomAdd(Room *room);
	int RoomUpdate(Room *room);
	int RoomDel(Room *room);
	int RoomDelAll();

	int RuleRead();
	int RuleAdd(Rule *rule, string data, int type);
	int RuleUpdateData(Rule *rule, string data);
	int RuleUpdateStatus(Rule *rule);
	int RuleUpdateAddr(Rule *rule);
	int RuleUpdateType(Rule *rule, int type);
	int RuleDel(Rule *rule);
	int RuleDelAll();

	int SceneBleRead();
	int SceneBleAdd(SceneBle *sceneBle);
	int SceneBleUpdate(SceneBle *sceneBle);
	int SceneBleUpdateRoom(SceneBle *sceneBle, string roomId);
	int SceneBleDel(SceneBle *sceneBle);
	int SceneBleDelAll();
	int SceneBleUpdateFavorite(SceneBle *scene);

#ifdef ESP_PLATFORM
	bool IsHaveDbV1();
	int OpenDbV1();
	int ConvertTableDevice();
	int ConvertTableDeviceAttribute();
	int ConvertTableDeviceBleChild();
	int ConvertTableDeviceInGroup();
	int ConvertTableDeviceInRoom();
	int ConvertTableDeviceInSceneBle();
	int ConvertTableGateway();
	int ConvertTableGroup();
	int ConvertTableRoom();
	int ConvertTableSceneBle();
	int ReadAll_V1(string table, void *listPtr, int (*Parse)(sqlite3_stmt *, void *));
	int ConvertTableRule();
	int ConvertTableSceneDelay();
	int EditTableDeviceInGroup();

#endif

#ifdef __ANDROID__
	int NotiRead();
	int NotiAdd(Noti *noti);
	int NotiUpdate(Noti *noti);
	int NotiDel(Noti *noti);
#endif
};

extern Db *database;
