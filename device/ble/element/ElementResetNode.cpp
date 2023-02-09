#include "ElementResetNode.h"
#include <Log.h>
#include <Util.h>
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ElementResetNode::ElementResetNode(Device *device, uint32_t addr) : Element(device, addr)
{
}

void ElementResetNode::ParseData(uint8_t *data, int len, Json::Value &jsonValue)
{
}

void ElementResetNode::BuildTelemetryValue(Json::Value &jsonValue)
{
}

bool ElementResetNode::Do()
{
	LOGD("DoTrigger reset");
	bleProtocol->ResetDev(addr);
	return true;
}
