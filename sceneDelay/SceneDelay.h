#pragma once

#include <string>
#include <vector>
#include "Object.h"
#include "json.h"
#include <unistd.h>
#include "SceneDelayOutput.h"

using namespace std;

class SceneDelay : public Object
{
private:
    Json::Value data;
    vector<SceneDelayOutput *> sceneDelayOutputs;

public:
    SceneDelay(string id, uint32_t addr, string name, Json::Value &data);
    ~SceneDelay();

    Json::Value GetData();
    void UpdateData(string data);
    void AddSceneDelayOutput(SceneDelayOutput *output);
    void DelAllSceneDelayOutput();
    void RunOutput();
};