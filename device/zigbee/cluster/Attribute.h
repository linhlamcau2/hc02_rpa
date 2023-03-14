#pragma once

#include <string>
#include "json.h"

using namespace std;

class Cluster;
class Attribute
{
protected:
	Cluster *cluster;

public:
	Attribute(Cluster *cluster);

	virtual void ParseData(uint8_t *data, int len, Json::Value &jsonValue) {}
	virtual bool CheckData(Json::Value &dataValue, bool &rs) { return false; }
};
