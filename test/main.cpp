#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <signal.h>
#include <fstream>
#include "json.h"
#include "Log.h"


#define TAG "MAIN"

using namespace std;


int main(int argc, char *argv[])
{
    while(1)
    {
        usleep(5000);
        LOGI("Start smh test");
    }
	
	return 0;
}
