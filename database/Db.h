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

#ifdef ESP_PLATFORM
#define DB_NAME "/spiffs/smh.sqlite"
#else
#define DB_NAME "/smh.sqlite"
#endif

using namespace std;

class Db
{

private:
	// TODO: 
	// mutex mtx;

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
	int GroupDel(int id);
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
	int RuleAdd(int id, string rule);
	int RuleUpdate(int id, string rule);
	int RuleDel(int id);

	int SceneBleRead();
	int DeviceInSceneBleAdd(SceneBle *scene, Device *device, Json::Value data);
	int DeviceInSceneBleDel(SceneBle *scene, Device *device, int epId);
	int SceneBleDel(SceneBle *scene);
};

extern Db *database;
