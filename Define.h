#pragma once

#ifndef MODEL
#ifdef __OPENWRT__
#define MODEL "RD_HC_FULL"
#elif defined(__ANDROID__)
#define MODEL "RD_HC_ANDROID"
#elif defined(ESP_PLATFORM)
#define MODEL "RD_HC_MINI"
#else
#define MODEL "RD_HC_V2"
#endif
#endif

#ifndef VERSION
#define VERSION 0.0.1
#endif

#ifndef DB_NAME
#ifdef __OPENWRT__
#define DB_NAME "/smh.sqlite"
#elif defined(__ANDROID__)
#define DB_NAME "/etc/smh/smh.sqlite"
#elif defined(ESP_PLATFORM)
#define DB_NAME "/spiffs/smh.sqlite"
#else
#define DB_NAME "./smh.sqlite"
#endif
#endif

#ifndef BLE_UART_PORT
#ifdef __OPENWRT__
#define BLE_UART_PORT "/dev/ttyS1"
#elif defined(__ANDROID__)
#define BLE_UART_PORT "/dev/ttyS5"
#elif defined(ESP_PLATFORM)
#else
#define BLE_UART_PORT "/dev/ttyUSB0"
#endif
#endif

#ifdef CONFIG_ENABLE_ZIGBEE
#include "ZigbeeProtocol.h"
#define ZIGBEE_UART_PORT "/dev/ttyS0"
#endif