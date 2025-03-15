#include "QrProtocol.h"
#include <stdlib.h>
#include <thread>
#include <functional>
#include <byteswap.h>
#include "Log.h"
#include "Util.h"
#include <string.h>
#include <algorithm>

QrProtocol *qrProtocol = NULL;

QrProtocol::QrProtocol(char *uartPort, int baudrate) : Uart(uartPort, baudrate, 100000)
{
	this->mac = "";
	this->startTest = false;
}

QrProtocol::~QrProtocol()
{
}

void QrProtocol::init()
{
	Uart::init();
	SLEEP_MS(100); // wait for thread start
}

std::string extractMac(const std::string &input)
{
	size_t pos = input.find("MAC"); // Tìm vị trí "MAC"
	if (pos != std::string::npos && pos + 15 <= input.length())
	{									  // Đảm bảo có đủ 12 ký tự sau "MAC"
		return input.substr(pos + 3, 12); // Lấy 12 ký tự sau "MAC"
	}
	return "Not found";
}

uint16_t getLast4HexAsUint16(const std::string &mac)
{
	if (mac.length() < 4)
	{
		return 0; // Tránh lỗi nếu chuỗi quá ngắn
	}

	std::string last4 = mac.substr(mac.length() - 4, 4); // Lấy 4 ký tự cuối
	uint16_t addr;

	std::stringstream ss;
	ss << std::hex << last4; // Chuyển chuỗi thành số hex
	ss >> addr;

	return addr;
}

int QrProtocol::OnMessage(unsigned char *data, int len)
{
	uint8_t *d = data;
	int l = len;
	std::string s(reinterpret_cast<char *>(d), l);
	LOGD("QrProtocol::OnMessage: %s", s.c_str());
	if (l > 0)
	{
		if (!this->startTest)
		{
			this->mac = extractMac(s);
			string tailMac = mac.substr(8, 12);
			this->addr = getLast4HexAsUint16(tailMac) - 0x8000;
			this->startTest = true;
			// LOGE("qr:mac %s", this->mac.c_str());
			// LOGE("qr:addr %d", this->addr);
		}
	}
	return l;
}