#include "gpioProtocol.h"
#include "Util.h"
#include "Log.h"
#include "Gateway.h"

int gpio_arr[] = {0, 37, 3, 2};
int stt_pin[4] ={0};
int pin_pow_k9b = 14;

int led_success = 17;

int gpio_arr_dhpt_test_tc_pos1[] = {19,18,17,16,15};
int gpio_arr_dhpt_test_tc_pos0[] = {14,37,3,2,0};
int gpio_arr_dhpt_test_smt_pos1[] = {19,18,17,16,15};
int gpio_arr_dhpt_test_smt_pos0[] = {14,37,3,2,0};

GPIOProtocol *gpioProtocol = NULL;

int GPIOProtocol ::gpio_get(int index)
{
    string cmd = "cat /sys/class/gpio/gpio" + to_string(gpio_arr[index]) + "/value";
    int status = std::stoi(Util::ExecuteCMD(cmd.c_str()));
    LOGE("CMD: %s; status: %d", cmd.c_str(), status);
    return status;
}

int GPIOProtocol ::gpio_get_pin_test_dhpt(int index, uint8_t type)
{
    string cmd;
    int status;
    if(type == PCBA_TEST_SMT_POS0)    
    {
        cmd = "cat /sys/class/gpio/gpio" + to_string(gpio_arr_dhpt_test_smt_pos0[index]) + "/value";
        status = !(std::stoi(Util::ExecuteCMD(cmd.c_str())));
    }
    else if(type == PCBA_TEST_SMT_POS1)    
    {
        cmd = "cat /sys/class/gpio/gpio" + to_string(gpio_arr_dhpt_test_smt_pos1[index]) + "/value";
        status = !(std::stoi(Util::ExecuteCMD(cmd.c_str())));
    }
    else if(type == PCBA_TEST_TC_POS0 )
    {
        cmd = "cat /sys/class/gpio/gpio" + to_string(gpio_arr_dhpt_test_tc_pos0[index]) + "/value";
        status = std::stoi(Util::ExecuteCMD(cmd.c_str()));
    }
    else if(type == PCBA_TEST_TC_POS1 )
    {
        cmd = "cat /sys/class/gpio/gpio" + to_string(gpio_arr_dhpt_test_tc_pos1[index]) + "/value";
        status = std::stoi(Util::ExecuteCMD(cmd.c_str()));
    }
    else
    {
        LOGE("Invalid type for gpio_get_pin_test_dhpt: %d", type);
        return -1;
    }
    LOGE("CMD: %s; status: %d", cmd.c_str(), status);
    return status;
}

int GPIOProtocol ::gpio_read(int index)
{
#ifdef ESP_PLATFORM
    if (index < 0 || index > num)
    {
        return -1;
    }
    return gpio_get_level(gpio_arr[index]);
#else
    return 0;
#endif
}

int GPIOProtocol ::dectect_gpio(int index)
{
#ifdef ESP_PLATFORM
    int stt = !(this->gpio_read(index));
    if (stt != state_gpio[index])
    {
        this->count[index]++;
        if (this->count[index] >= CYCLE_ACTIVE)
        {
            this->count[index] = 0;
            state_gpio[index] = stt;
            LOGI("GPIO %d: %d", index, stt);
            // if(gateway != NULL)
            //     gateway->CloudPublish("relay: " + to_string(index+1) + " "+ to_string(stt));
            return 1;
        }
    }
#else

#endif
    return 0;
}

int GPIOProtocol ::handle_gpio(int index)
{
    return 0;
}

static void gpio_task(void *pvParameters)
{
#ifdef ESP_PLATFORM
    GPIOProtocol *gpioProtocol = (GPIOProtocol *)pvParameters;
    gpioProtocol->on_gpio();
#else

#endif
}

void GPIOProtocol ::on_gpio()
{
#ifdef ESP_PLATFORM
    while (1)
    {
        for (int i = 0; i < num; i++)
        {
            if (dectect_gpio(i))
            {
                handle_gpio(i);
            }
        }
        SLEEP_MS(5);
    }
    vTaskDelete(NULL);
#else

#endif
}

void GPIOProtocol ::gpio_init()
{
    Util::ExecuteCMD("echo 0 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio0/direction");

    Util::ExecuteCMD("echo 37 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio37/direction");

    Util::ExecuteCMD("echo 3 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio3/direction");

    Util::ExecuteCMD("echo 2 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio2/direction");

    Util::ExecuteCMD("echo 14 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio14/direction");

    Util::ExecuteCMD("echo 15 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio15/direction");

    Util::ExecuteCMD("echo 16 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio16/direction");

    Util::ExecuteCMD("echo 17 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio17/direction");

    Util::ExecuteCMD("echo 18 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio18/direction");

    Util::ExecuteCMD("echo 19 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio19/direction");

    Util::ExecuteCMD("echo 1 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio1/direction");

    Util::ExecuteCMD("echo 1 > /sys/class/gpio/gpio1/value");

    Util::ExecuteCMD("echo 4 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio4/direction");

    Util::ExecuteCMD("echo 0 > /sys/class/gpio/gpio4/value");

    Util::ExecuteCMD("echo 5 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio5/direction");

    Util::ExecuteCMD("echo 0 > /sys/class/gpio/gpio5/value");

}

void restart_chip_tlsr8253(int pos)
{
    if(pos == 0)
    {
        Util::ExecuteCMD("echo 0 > /sys/class/gpio/gpio4/value");
        SLEEP_MS(1000);
        Util::ExecuteCMD("echo 1 > /sys/class/gpio/gpio4/value");
        SLEEP_MS(1000);
    }
    else 
    {
        Util::ExecuteCMD("echo 0 > /sys/class/gpio/gpio5/value");
        SLEEP_MS(1000);
        Util::ExecuteCMD("echo 1 > /sys/class/gpio/gpio5/value");
        SLEEP_MS(1000);
    }
}

void start_process_test_smt(int pos)
{
    Util::ExecuteCMD("echo 0 > /sys/class/gpio/gpio1/value");
    SLEEP_MS(1000);
    if(pos == 0) Util::ExecuteCMD("echo 1 > /sys/class/gpio/gpio4/value");
    else Util::ExecuteCMD("echo 1 > /sys/class/gpio/gpio5/value");
    SLEEP_MS(1000);
}

void end_process_test_smt(int pos)
{
    if(pos == 0) Util::ExecuteCMD("echo 0 > /sys/class/gpio/gpio4/value");
    else Util::ExecuteCMD("echo 0 > /sys/class/gpio/gpio5/value");
    SLEEP_MS(1000);
    Util::ExecuteCMD("echo 1 > /sys/class/gpio/gpio1/value");
    SLEEP_MS(1000);
}

void GPIOProtocol ::gpio_supply_power_k9b()
{
#ifdef ESP_PLATFORM
    gpio_set_level(pin_pow_k9b, 0);
    gpio_set_level(pin_up_k9b, 0);
    gpio_set_level(pin_down_k9b, 1);
    SLEEP_MS(70);
    gpio_set_level(pin_down_k9b, 0);
    SLEEP_MS(70);
    gpio_set_level(pin_up_k9b, 1);
    SLEEP_MS(70);
    gpio_set_level(pin_up_k9b, 0);
    SLEEP_MS(600);
    gpio_set_level(pin_pow_k9b, 1);
#else
    string cmd = "echo 1 > /sys/class/gpio/gpio" + to_string(pin_pow_k9b) + "/value";
    Util::ExecuteCMD(cmd.c_str());
    cmd = "echo 1 > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness";
    Util::ExecuteCMD(cmd.c_str());
    cmd = "echo 0 > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness";
    Util::ExecuteCMD(cmd.c_str());

    SLEEP_MS(70);
    cmd = "echo 1 > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness";
    Util::ExecuteCMD(cmd.c_str());

    SLEEP_MS(70);
    cmd = "echo 0 > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness";
    Util::ExecuteCMD(cmd.c_str());

    SLEEP_MS(70);
    cmd = "echo 1 > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness";
    Util::ExecuteCMD(cmd.c_str());

    SLEEP_MS(600);
    cmd = "echo 0 > /sys/class/gpio/gpio" + to_string(pin_pow_k9b) + "/value";
    Util::ExecuteCMD(cmd.c_str());
#endif
}

void GPIOProtocol ::set_led_success()
{
#ifdef ESP_PLATFORM
    gpio_set_level(led_success, 1);
    gpio_set_level(led_fail, 0);
#else
    string cmd = "echo 1 > /sys/class/leds/linkit-smart-7688:orange:internet/brightness";
    string cmd1 = "echo 1 > /sys/class/gpio/gpio" + to_string(led_success) + "/value";
    Util::ExecuteCMD(cmd.c_str());
    Util::ExecuteCMD(cmd1.c_str());
#endif
}

void GPIOProtocol ::set_led_fail()
{
#ifdef ESP_PLATFORM
    gpio_set_level(led_success, 0);
    gpio_set_level(led_fail, 1);
#else
    string cmd = "echo 0 > /sys/class/leds/linkit-smart-7688:orange:internet/brightness";
    string cmd1 = "echo 0 > /sys/class/gpio/gpio" + to_string(led_success) + "/value";
    Util::ExecuteCMD(cmd.c_str());
    Util::ExecuteCMD(cmd1.c_str());
#endif
}

void GPIOProtocol ::set_led_warning(uint8_t stt)
{
// #ifdef ESP_PLATFORM
//     gpio_set_level(led_warning, stt);
// #else
//     string cmd1 = "echo " + to_string(!stt) + " > /sys/class/leds/linkit-smart-7688:orange:service/brightness";
//     Util::ExecuteCMD(cmd1.c_str());
// #endif
}

void GPIOProtocol ::reset_led_in_proc()
{
#ifdef ESP_PLATFORM
    gpio_set_level(led_success, 0);
    gpio_set_level(led_fail, 0);
#else
    string cmd = "echo 1 > /sys/class/leds/linkit-smart-7688:orange:internet/brightness";
    string cmd1 = "echo 0 > /sys/class/gpio/gpio" + to_string(led_success) + "/value";
    Util::ExecuteCMD(cmd.c_str());
    Util::ExecuteCMD(cmd1.c_str());
#endif
}

void GPIOProtocol ::set_mode_input()
{
#ifdef ESP_PLATFORM
    for (int i = 0; i < num; i++)
    {
        gpio_reset_pin(gpio_arr[i]);
        gpio_set_direction(gpio_arr[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(gpio_arr[i], GPIO_PULLUP_ONLY);
    }
#else

#endif
}