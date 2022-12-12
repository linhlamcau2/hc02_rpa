#pragma once

#define BUFFER_SIZE 1024

#include <unistd.h>
#include <thread>
#include <functional>
#include <termios.h>
#include <mutex>

using namespace std;

class Uart
{
private:
	char *port;
	thread *uartThread;
	mutex mtx;

public:
	volatile int fd;
	int timeout;
	unsigned char rx_buf[BUFFER_SIZE];

	Uart(char *port, int timeout);
	virtual ~Uart();

	int Open(int baudrate);
	int Close();
	int ChangeBaudrate(int baudrate);
	ssize_t Read(void *buf, size_t count);
	ssize_t Write(const void *buf, size_t count);

	virtual void OnMessage(unsigned char *data, int len);
};
