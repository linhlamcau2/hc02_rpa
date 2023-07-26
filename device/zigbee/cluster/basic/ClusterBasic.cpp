#include "ClusterBasic.h"
#include "Log.h"

ClusterBasic::ClusterBasic(Device *device, string modelKey) : Cluster(CLUSTER_GENERAL_BASIC, device)
{
	attributeModel = new AttributeModel(this, modelKey);
	attributes.push_back(attributeModel);
}
