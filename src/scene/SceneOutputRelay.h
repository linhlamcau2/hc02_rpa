#pragma once

#include <json.h>
#include "Device.h"
#include "SceneOutput.h"

using namespace std;

class SceneOutputRelay : public SceneOutput
{
private:
	int relay;
	int value;

public:
	SceneOutputRelay(int relay, int value);
	~SceneOutputRelay();
	
	void RunOutput();
};