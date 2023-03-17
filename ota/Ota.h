#pragma once

#include <string>

using namespace std;

namespace Ota
{
	void init();
	int startOta(string name, string url, string sum);
}