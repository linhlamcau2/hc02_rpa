#pragma once
#include <string>
#include <string.h>
#include <sqlite3.h>
#include <vector>
#include <mutex>
#include <Device.h>
#include <Group.h>
#include <Gateway.h>

#ifdef CONFIG_ENABLE_MODBUS
#include "ModbusParameter.h"
#endif

#define DB_NAME "/smh.sqlite"

using namespace std;

class Db
{

private:
	mutex mtx;

	int Sqlite_Exec(string &sql);
	int ReadAll(string table, void *listPtr, int (*Parse)(sqlite3_stmt *, void *));

public:
	Db();
	~Db() {}

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

	int DeviceSceneRead();
	int DeviceSceneAdd(string mac, string schedule);
	int DeviceSceneAdd(Device *device, string schedule);
	int DeviceSceneDel(string mac, string schedule);
	int DeviceSceneDel(Device *device, string schedule);

	int SceneRead();
	int SceneAdd(int id, string scene);
	int SceneUpdate(int id, string scene);
	int SceneDel(int id);

#ifdef CONFIG_ENABLE_MODBUS
	int ModbusDeviceRead();
	int ModbusDeviceAdd(string mac, string serialPort, int baudrate, int modbusAddress, int scanRate, int timeout);
	int ModbusDeviceUpdate(string mac, string serialPort, int baudrate, int modbusAddress, int scanRate, int timeout);
	int ModbusDeviceDel(string mac);
	int ModbusDeviceDelAll();

	int ModbusParameterRead();
	int ModbusParameterAdd(ModbusParameter *modbusParameter, string mac);
	int ModbusParameterUpdate(ModbusParameter *modbusParameter);
	int ModbusParameterDel(int id);
#endif
};

extern Db *database;