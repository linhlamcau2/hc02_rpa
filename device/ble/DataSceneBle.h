#pragma once

#include <iostream>
#include <string>
#include "json.h"
namespace DataSceneBle
{
    void InitDataSceneBle();
    Json::Value GetDataSceneBle();
    Json::Value GetDataDeviceInScene(uint32_t type, int idScene);
}