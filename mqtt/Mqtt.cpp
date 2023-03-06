#include "Mqtt.h"
#include "mosquitto.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <Log.h>
#include <Util.h>

#define TAG "Mqtt"

using namespace mosqpp;

Mqtt::Mqtt(string host, int port, string client_id, string username, string password, int keepalive, string willset_topic, string willset_payload) : mosquittopp(client_id.c_str())
{
	this->host = host;
	this->port = port;
	this->client_id = client_id;
	this->username = username;
	this->password = password;
	this->keepalive = keepalive;
	this->willset_topic = willset_topic;
	this->willset_payload = willset_payload;
	connected = false;
	reconnected = false;
}

Mqtt::~Mqtt()
{
	connected = false;
	reconnected = false;
}

void Mqtt::init()
{
}

void Mqtt::SetServer(string host, int port, string client_id, string username, string password, int keepalive)
{
	this->host = host;
	this->port = port;
	this->client_id = client_id;
	this->username = username;
	this->password = password;
	this->keepalive = keepalive;
}

void Mqtt::SetWillset(string willset_topic, string willset_payload)
{
	this->willset_topic = willset_topic;
	this->willset_payload = willset_payload;
}

int Mqtt::Connect()
{
	LOGD("Connect host %s, port %d", host.c_str(), port);
	if (!username.empty() || !password.empty())
	{
		if (username_pw_set(username.c_str(), password.c_str()) != MOSQ_ERR_SUCCESS)
		{
			LOGE("setting passwd failed");
		}
	}

	if (willset_topic.size() > 0 && willset_payload.size() > 0)
	{
		LOGD("Willset topic: %s, payload:\n%s", willset_topic.c_str(), willset_payload.c_str());
		will_set(willset_topic.c_str(), willset_payload.size(), willset_payload.c_str());
	}

	int result = loop_start();
	if (result == MOSQ_ERR_SUCCESS)
	{
		result = connect_async(host.c_str(), port, keepalive);
		if (result != MOSQ_ERR_SUCCESS)
		{
			LOGW("connect_async failed code %d, err %s", result, mosqpp::strerror(result));
		}
	}
	else
	{
		LOGE("loop_start failed code %d, err %s", result, mosqpp::strerror(result));
	}
	return result;
}

int Mqtt::Reconnect()
{
	return reconnect_async();
}

int Mqtt::removeObjectFromVector(vector<MQTTPubSub *> *mqttPubSubs, MQTTPubSub *mqttPubSub)
{
	for (vector<MQTTPubSub *>::iterator it = (*mqttPubSubs).begin(); it != (*mqttPubSubs).end(); ++it)
	{
		if (*it == mqttPubSub)
		{
			(*mqttPubSubs).erase(it);
			return 1;
		}
	};
	return 0;
}

void Mqtt::SubscribeList()
{
	for (auto topic : topicList)
	{
		if (!topic.empty())
			subscribe(NULL, topic.c_str());
	}
}

int Mqtt::Subscribe(string topic, int maxTime, int duration)
{
	LOGD("Subscribe: %s", topic.c_str());
	time_t currentTime;
	mtx.lock();
	MQTTPubSub mqttSubscribe;
	mqttSubscribe.setState(false);
	if (mqttSubscribes.size() >= MAX_BUFFER_SIZE)
	{
		LOGW("mqttSubscribes buffer size: %d", (int)mqttSubscribes.size());
		mqttSubscribes.erase(mqttSubscribes.begin());
	}
	mqttSubscribes.push_back(&mqttSubscribe);
	int ret = subscribe(&mqttSubscribe.id, topic.c_str());
	if (ret != MOSQ_ERR_SUCCESS)
	{
		removeObjectFromVector(&mqttSubscribes, &mqttSubscribe);
		mtx.unlock();
		return ret;
	}
	for (int i = 0; i < maxTime; i++)
	{
		currentTime = time(NULL);
		while (time(NULL) < currentTime + duration)
		{
			if (mqttSubscribe.getState() == true)
			{
				LOGD("Subscribe topic: %s OK", topic.c_str());
				removeObjectFromVector(&mqttSubscribes, &mqttSubscribe);
				mtx.unlock();
				return MOSQ_ERR_SUCCESS;
			}
			usleep(1000);
		}
		ret = subscribe(&mqttSubscribe.id, topic.c_str());
		LOGI("Resubscribes topic: %s, ret: %d", topic.c_str(), ret);
	}
	removeObjectFromVector(&mqttSubscribes, &mqttSubscribe);
	LOGW("Subscribe topic: %s time out", topic.c_str());
	mtx.unlock();
	return MQTT_ERR_TIMEOUT;
}

int Mqtt::Unsubscribe(string topic, int maxTime, int duration)
{
	time_t currentTime;
	mtx.lock();
	MQTTPubSub mqttUnsubscribe;
	mqttUnsubscribe.setState(false);
	if (mqttUnsubscribes.size() >= MAX_BUFFER_SIZE)
	{
		LOGW("mqttUnsubscribes buffer size: %d", (int)mqttUnsubscribes.size());
		mqttUnsubscribes.erase(mqttUnsubscribes.begin());
	}
	mqttUnsubscribes.push_back(&mqttUnsubscribe);
	int ret = unsubscribe(&mqttUnsubscribe.id, topic.c_str());
	if (ret != MOSQ_ERR_SUCCESS)
	{
		removeObjectFromVector(&mqttUnsubscribes, &mqttUnsubscribe);
		mtx.unlock();
		return ret;
	}
	for (int i = 0; i < maxTime; i++)
	{
		currentTime = time(NULL);
		while (time(NULL) < currentTime + duration)
		{
			if (mqttUnsubscribe.getState() == true)
			{
				LOGD("Unsubscribes topic: %s OK", topic.c_str());
				removeObjectFromVector(&mqttUnsubscribes, &mqttUnsubscribe);
				mtx.unlock();
				return MOSQ_ERR_SUCCESS;
			}
			usleep(1000);
		}
		ret = unsubscribe(&mqttUnsubscribe.id, topic.c_str());
		LOGI("Unsubscribes topic: %s, ret: %d", topic.c_str(), ret);
	}
	removeObjectFromVector(&mqttUnsubscribes, &mqttUnsubscribe);
	LOGW("Unsubscribes topic: %s time out", topic.c_str());
	mtx.unlock();
	return MQTT_ERR_TIMEOUT;
}

int Mqtt::Publish(string topic, string payload, int maxTime, int duration)
{
	time_t currentTime;
	if (!connected)
		return -1;
	MQTTPubSub mqttPublish;
	mqttPublish.setState(false);
	LOGV("Publish topic: %s, payload:\n%s", topic.c_str(), payload.c_str());
	mtx.lock();
	if (mqttPublishs.size() >= MAX_BUFFER_SIZE)
	{
		LOGW("mqttSubscribes buffer size: %d", (int)mqttPublishs.size());
		mqttPublishs.erase(mqttPublishs.begin());
	}
	mqttPublishs.push_back(&mqttPublish);
	LOGV("publish");
	int ret = publish(&mqttPublish.id, topic.c_str(), payload.size(), payload.c_str());
	if (ret != MOSQ_ERR_SUCCESS)
	{
		removeObjectFromVector(&mqttPublishs, &mqttPublish);
		LOGW("Publish error: %d", ret);
		mtx.unlock();
		return ret;
	}
	for (int i = 0; i < maxTime; i++)
	{
		currentTime = time(NULL);
		while (time(NULL) < currentTime + duration)
		{
			if (mqttPublish.getState() == true)
			{
				LOGV("Publish topic: %s OK", topic.c_str());
				removeObjectFromVector(&mqttPublishs, &mqttPublish);
				mtx.unlock();
				return MOSQ_ERR_SUCCESS;
			}
			usleep(1000);
		}
		publish(&mqttPublish.id, topic.c_str(), payload.size(), payload.c_str());
	}
	removeObjectFromVector(&mqttPublishs, &mqttPublish);
	LOGW("Publish topic: %s time out", topic.c_str());
	mtx.unlock();
	return MQTT_ERR_TIMEOUT;
}

int Mqtt::Publish(string topic, char *payload, int payloadLen)
{
	return publish(NULL, topic.c_str(), payloadLen, payload);
}

bool Mqtt::isConnected()
{
	return connected;
}

void Mqtt::on_connect(int rc)
{
	connected = true;
	reconnected = true;
	try
	{
		auto onMessageFunc = bind(&Mqtt::OnConnect, this, placeholders::_1, placeholders::_2);
		thread myThread(onMessageFunc, true, reconnected);
		myThread.detach();
		auto subscribeListFunc = bind(&Mqtt::SubscribeList, this);
		thread subscribeListThread(subscribeListFunc);
		subscribeListThread.detach();
	}
	catch (...)
	{
		LOGE("onMessageCallbackFunc error");
	}
}

void Mqtt::on_disconnect(int rc)
{
	LOGW("Disconnected with code %d, err: %s", rc, mosqpp::strerror(rc));
	connected = false;
	try
	{
		auto onMessageFunc = bind(&Mqtt::OnConnect, this, placeholders::_1, placeholders::_2);
		thread myThread(onMessageFunc, false, reconnected);
		myThread.detach();
	}
	catch (...)
	{
		LOGE("onMessageCallbackFunc error");
	}
}

void Mqtt::on_publish(int mid)
{
	LOGV("Published message with id: %d", mid);
	for (auto &mqttPublish : mqttPublishs)
	{
		if (mqttPublish->id == mid)
		{
			mqttPublish->setState(true);
		}
	}
}

void Mqtt::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
	LOGV("Subscribe message with id: %d", mid);
	for (auto &mqttSubscribe : mqttSubscribes)
	{
		if (mqttSubscribe->id == mid)
		{
			mqttSubscribe->setState(true);
		}
	}
}

void Mqtt::on_unsubscribe(int mid)
{
	LOGV("Unsubscribe message with id: %d", mid);
	for (auto &mqttUnsubscribe : mqttUnsubscribes)
	{
		if (mqttUnsubscribe->id == mid)
		{
			mqttUnsubscribe->setState(true);
		}
	}
}

void Mqtt::on_message(const struct mosquitto_message *message)
{
	string topic = string(message->topic);
	string payload = string((char *)message->payload);
	try
	{
		auto onMessageFunc = bind(&Mqtt::OnMessage, this, placeholders::_1, placeholders::_2);
		thread myThread(onMessageFunc, topic, payload);
		myThread.detach();
	}
	catch (...)
	{
		LOGE("onMessageCallbackFunc error");
	}
}

void Mqtt::OnMessage(string topic, string payload)
{
	// LOGD("OnMessage topic: %s, payload: %s", topic.c_str(), payload.c_str());
	ActionCallback *actionCallback;
	int getCallback = findActionCallbackFuncFromTopic(topic, &actionCallback);
	if (getCallback == 1)
	{
		if (actionCallback->getType() == 1)
		{
			actionCallback->actionCallbackFuncType1(topic, payload);
		}
		else if (actionCallback->getType() == 2)
		{
			actionCallback->actionCallbackFuncType2(topic, payload);
		}
	}
}

void Mqtt::addActionCallback(ActionCallbackFuncType1 actionCallbackFuncType1, string topic)
{
	ActionCallback actionCallback(actionCallbackFuncType1, topic);
	actionCallbacks.push_back(actionCallback);
	if (connected)
		Subscribe(topic);
	topicList.push_back(topic);
}

void Mqtt::addActionCallback(ActionCallbackFuncType2 actionCallbackFuncType2, string topic)
{
	ActionCallback actionCallback(actionCallbackFuncType2, topic);
	actionCallbacks.push_back(actionCallback);
	if (connected)
		Subscribe(topic);
	topicList.push_back(topic);
}

int checkMqttTopic(string retrieveTopic, string registerTopic)
{
	vector<string> retrieveList = Util::splitString(retrieveTopic, '/');
	vector<string> registerList = Util::splitString(registerTopic, '/');
	for (size_t i = 0; i < retrieveList.size(); i++)
	{
		if (registerList.size() < i)
			return 0;
		if (registerList.at(i) == "#")
			return 1; // OK
		if (registerList.at(i) == "+")
			continue;
		if (registerList.at(i) != retrieveList.at(i))
			return 0;
	}
	if (registerList.size() == retrieveList.size())
		return 1; // OK
	return 0;
}

int Mqtt::findActionCallbackFuncFromTopic(string topic, ActionCallback **actionCallback)
{
	for (auto &action : actionCallbacks)
	{
		if (checkMqttTopic(topic, action.getTopic()))
		{
			*actionCallback = &action;
			return 1;
		}
	}
	return -1;
}
