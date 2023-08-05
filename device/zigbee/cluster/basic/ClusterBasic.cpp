#include "ClusterBasic.h"
#include "Log.h"

ClusterBasic::ClusterBasic(Device *device) : Cluster(ZCL_CLUSTER_GEN_BASIC, device)
{
	attributeModelId = new AttributeModelId(this, "modelId");
	attributes.push_back(attributeModelId);
	attributeAppVer = new AttributeAppVer(this, "appVer");
	attributes.push_back(attributeAppVer);
}
