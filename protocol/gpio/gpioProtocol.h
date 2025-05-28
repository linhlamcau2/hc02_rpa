#pragma once

#include <string>
#include <stdint.h>
#include <vector>

#define CYCLE_ACTIVE 5

using namespace std;

class GPIOProtocol
{
private:
    int num;
    vector<int> state_gpio;
    vector<int> count;

    int gpio_read(int index);
    int dectect_gpio(int index);
    int handle_gpio(int index);
    void process_rpa();
    void button_thread_func();
public:
    // GPIOProtocol();
    // ~GPIOProtocol();
    void gpio_init();
    int gpio_get(int index);
    void on_gpio();
    void set_led_success();
    void set_led_fail();
    void set_led_warning(uint8_t stt);
    void reset_led_in_proc();
    void set_mode_input();
    void k9b_press(int id_but);
};

extern GPIOProtocol *gpioProtocol;