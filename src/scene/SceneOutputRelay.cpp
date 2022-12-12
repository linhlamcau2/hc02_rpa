#include "SceneOutputRelay.h"
#include "BATProtocol.h"
#include "Log.h"

SceneOutputRelay::SceneOutputRelay(int relay, int value)
{
	this->relay = relay;
	this->value = value;
}

SceneOutputRelay::~SceneOutputRelay()
{
	LOGI("~SceneOutputRelay");
}

void SceneOutputRelay::RunOutput()
{
#if BAT
	uint8_t relayState;
	batProtocol->ControlRelay(relay, value, relayState);
#endif
}
