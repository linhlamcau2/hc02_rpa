#pragma once

using namespace std;

class Scene;
class SceneInput
{
protected:
	bool isAvailable;
	Scene *scene;

public:
	virtual ~SceneInput() {}
	virtual bool Check() { return isAvailable; }
};