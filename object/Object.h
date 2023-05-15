#pragma once

#include <string>
#include <stdint.h>

using namespace std;

class Object
{
protected:
	string id;
	string name;
	uint32_t addr;

public:
	Object(string id, uint32_t addr, string name);
	~Object();

	string GetId();
	void SetId(string id);
	string GetName();
	void SetName(string name);
	uint32_t GetAddr();
	void SetAddr(uint32_t addr);
};
