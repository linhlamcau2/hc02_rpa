#include "QrProtocol.h"
#include <stdlib.h>
#include <thread>
#include <functional>
#include <byteswap.h>
#include "Log.h"
#include "Util.h"
#include <string.h>
#include <algorithm>
#include "Config.h"

QrProtocol *qrProtocol = NULL;

#ifdef ESP_PLATFORM
QrProtocol::QrProtocol(uart_port_t num, int txPin, int rxPin, int baudrate) : Uart(num, txPin, rxPin, baudrate)
#else
QrProtocol::QrProtocol(char *uartPort, int baudrate) : Uart(uartPort, baudrate, 100000)
#endif
{
	this->mac = "";
	this->prod_code = "";
	this->prod_num = "";
	this->serial = "";
	this->startTest = false;
	this->mac_k9b = config->GetMacK9B();
	this->mac_k9b_int = strtoul(this->mac_k9b.c_str(), NULL, 16);
	LOGI("K9B MAC int: %X", this->mac_k9b_int);
	this->isMac_k9b = false;
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

std::string extractCnNumber(const std::string& input) {
    size_t pos = input.find("CN.");
    if (pos != std::string::npos && pos + 5 <= input.length()) {
        return input.substr(pos + 4, 1); // Lấy 2 ký tự sau "CN."
    }
    return ""; // Trả về chuỗi rỗng nếu không tìm thấy
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

int countHyphens(const std::string &str)
{
	int count = 0;
	for (char c : str)
	{
		if (c == '-')
		{
			count++;
		}
	}
	return count;
}

bool parseQRCode(const std::string& code, std::string& prod_num, std::string& prod_code, std::string& serial, std::string& mac) {
	int qrLength = code.length();
	LOGD("QrProtocol::len: %d", qrLength);
    if (qrLength != 123) return false;  // check length

	prod_num   = code.substr(qrLength-43, 13);
	prod_code   = code.substr(qrLength-30, 8);
	serial = code.substr(qrLength-22, 9);

    if (code[qrLength-13] != '-') return false;  // check char '-'

    mac = code.substr(qrLength-12, 12);
    return true;
}

int QrProtocol::OnMessage(unsigned char *data, int len)
{
	if (this->startTest)
		return len; // ignore messages after test started

	if(data[len - 1] != 0x0D)
	{
		return len; // ignore messages without 0x0D at the end
	}
	uint8_t *d = data;
	int l = len -1 ;
	std::string s(reinterpret_cast<char *>(d), l);
	LOGD("QrProtocol::OnMessage: %s", s.c_str());
	if (l > 0)
	{
		// string prefix = s.substr(0, 8);
		// if (prefix == "CTCU.BLE")
		// {
		// 	if (countHyphens(s) == 4)
		// 	{
		// 		if (!this->startTest)
		// 		{
		// 			string p = extractMac(s);
		// 			// if((this->mac).compare(p) !=0)
		// 			string type = extractCnNumber(s);
		// 			LOGI("Type scan %s", type.c_str());
		// 			if(1)
		// 			{
		// 				this->mac = p;
		// 				string tailMac = mac.substr(8, 12);
		// 				LOGI("Check", "OnMessage: %s", tailMac.c_str());
		// 				uint16_t mac_tail = getLast4HexAsUint16(tailMac);
		// 				this->addr = (mac_tail > 0x8000 ) ? (mac_tail - 0x8000) : mac_tail ;
		// 				this->type_dev = std::stoi(type);
		// 				this->startTest = true;
		// 			}
		// 			LOGE("qr:mac %s", this->mac.c_str());
		// 			LOGE("qr:addr %d", this->addr);
		// 			LOGE("qr:type %d", this->type_dev);
		// 		}
		// 	}
		// }
		// else if (prefix == "KDKP.BLE")
		// {
		// 	size_t pos = s.find("MAC");
		// 	if (pos != string::npos)
		// 	{
		// 		string macAddress = s.substr(pos + 3);
		// 		if(macAddress.size() >= 8)
		// 		{
		// 			macAddress = macAddress.substr(0, 8);
		// 			this->mac_k9b = macAddress;
		// 			config->SetMacK9B(macAddress);
		// 			this->mac_k9b_int = strtoul(macAddress.c_str(), NULL, 16);
		// 			LOGI("QrProtocol::OnMessage: K9B MAC int: %X", this->mac_k9b_int);
		// 			this->isMac_k9b = true;
		// 		}
		// 	}
		// }
		if(parseQRCode(s, this->prod_num, this->prod_code, this->serial, this->mac))
		{
			LOGI("QrProtocol::OnMessage: Product Number: %s", this->prod_num.c_str());
			LOGI("QrProtocol::OnMessage: Product Code: %s", this->prod_code.c_str());
			LOGI("QrProtocol::OnMessage: Serial: %s", this->serial.c_str());
			LOGI("QrProtocol::OnMessage: MAC: %s", this->mac.c_str());

			
			uint16_t mac_tail = getLast4HexAsUint16(this->mac);
			this->addr = (mac_tail > 0x8000 ) ? (mac_tail - 0x8000) : mac_tail;
			this->startTest = true;

		}
		else
		{
			LOGE("QrProtocol::OnMessage: Invalid QR code");
		}
	}
	return l;
}
