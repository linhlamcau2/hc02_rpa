#include "TimerSchedule.h"
#include <Log.h>
#include <unistd.h>
#include <Util.h>

static void run(TimerSchedule *timerSchedule);

TimerSchedule *timerSchedule = NULL;

Timer::Timer(int index, int time, TimerCallbackFunc timerCallbackFunc)
{
	this->index = index;
	this->time = time;
	this->timerCallbackFunc = timerCallbackFunc;
}

int Timer::GetIndex()
{
	return index;
}

bool Timer::IsAtTime(int time)
{
	if (this->time == time)
		return true;
	return false;
}

void Timer::run()
{
	LOGD("run");
	try
	{
		thread runThread(timerCallbackFunc);
		runThread.detach();
	}
	catch (...)
	{
		LOGE("Timer run error");
	}
}

TimerSchedule::TimerSchedule()
{
	index = 0;
}

void TimerSchedule::init()
{
	LOGI("Start Timer init");
	runThread = new thread(run, this);
	runThread->detach();
}

static void run(TimerSchedule *timerSchedule)
{
	LOGI("Start Timer run");
	int currentTimer, oldTimer = 0;
	while (1)
	{
		currentTimer = Util::GetCurrentTimer();
		if (currentTimer != oldTimer)
		{
			LOGD("h:m: %d-%d", currentTimer, currentTimer);
			timerSchedule->mtx.lock();
			for (auto &timer : timerSchedule->timerList)
			{
				if (timer->IsAtTime(currentTimer))
				{
					timer->run();
				}
			}
			timerSchedule->mtx.unlock();
			oldTimer = currentTimer;
		}
		usleep(500000);
	}
}

int TimerSchedule::RegisterTimer(string timerStr, TimerCallbackFunc timerCallbackFunc)
{
	int timer = Util::ConvertStrTimeToInt(timerStr);
	return RegisterTimer(timer, timerCallbackFunc);
}

int TimerSchedule::RegisterTimer(int time, TimerCallbackFunc timerCallbackFunc)
{
	//TODO: sap xep list thoi gian
	Timer *timer = new Timer(++index, time, timerCallbackFunc);
	mtx.lock();
	timerList.push_back(timer);
	mtx.unlock();
	return index;
}

int TimerSchedule::UnregisterTimer(int index)
{
	mtx.lock();
	for (auto &timer : timerList)
	{
		if (timer->GetIndex() == index)
		{
			timerList.erase(remove(timerList.begin(), timerList.end(), timer), timerList.end());
		}
	}
	mtx.unlock();
	return 0;
}
