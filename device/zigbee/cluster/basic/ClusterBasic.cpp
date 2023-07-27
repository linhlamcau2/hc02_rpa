#include "ClusterBasic.h"
#include "Log.h"

ClusterBasic::ClusterBasic(Device *device, string modelKey) : Cluster(ZCL_CLUSTER_GEN_BASIC, device)
{
	attributeModel = new AttributeModel(this, modelKey);
	attributes.push_back(attributeModel);
}
