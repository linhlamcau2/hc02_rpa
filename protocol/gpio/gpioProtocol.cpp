#include "gpioProtocol.h"
#include "Util.h"
#include "Log.h"
#include "Gateway.h"

#ifdef ESP_PLATFORM
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

gpio_num_t gpio_arr[] = {GPIO_NUM_27, GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_14};
gpio_num_t pin_pow_k9b = GPIO_NUM_25;
gpio_num_t pin_down_k9b = GPIO_NUM_26;
gpio_num_t pin_up_k9b = GPIO_NUM_33;

gpio_num_t led_success = GPIO_NUM_32;
gpio_num_t led_fail = GPIO_NUM_18;
gpio_num_t led_warning = GPIO_NUM_19;
gpio_num_t gpio_arr[] = {0, 1, 2, 3};
#else
int gpio_arr[] = {0, 37, 3, 2};
int stt_pin[4] ={0};
int pin_pow_k9b = 14;

int led_success = 17;

#endif
GPIOProtocol *gpioProtocol = NULL;

int GPIOProtocol ::gpio_get(int index)
{
#ifndef ESP_PLATFORM
    string cmd = "cat /sys/class/gpio/gpio" + to_string(gpio_arr[index]) + "/value";
    int status = std::stoi(Util::ExecuteCMD(cmd.c_str()));
    
    if(stt_pin[index] != status)
    {
        stt_pin[index] = status;
        LOGE("CMD: %s; status: %d", cmd.c_str(), status);
    }
    return status;
#else
    if (index < 0 || index > num)
    {
        return -1;
    }
    return state_gpio[index];
#endif
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
    // num = sizeof(gpio_arr) / sizeof(gpio_arr[0]);
    // state_gpio.resize(num, 0);
    // count.resize(num, 0);
#ifdef ESP_PLATFORM
    for (int i = 0; i < num; i++)
    {
        gpio_set_direction(gpio_arr[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(gpio_arr[i], GPIO_PULLUP_ONLY);
    }
    gpio_set_direction(pin_pow_k9b, GPIO_MODE_OUTPUT);
    // gpio_pulldown_en(pin_pow_k9b);
    gpio_set_level(pin_pow_k9b, 1);
    gpio_set_direction(pin_down_k9b, GPIO_MODE_OUTPUT);
    // gpio_pulldown_en(pin_down_k9b);
    gpio_set_level(pin_down_k9b, 1);
    gpio_set_direction(pin_up_k9b, GPIO_MODE_OUTPUT);
    // gpio_pulldown_en(pin_up_k9b);
    gpio_set_level(pin_up_k9b, 1);

    gpio_set_direction(led_success, GPIO_MODE_OUTPUT); // led_warning
    gpio_set_level(led_success, 1);
    gpio_set_direction(led_fail, GPIO_MODE_OUTPUT);
    gpio_set_level(led_fail, 1);
    gpio_set_direction(led_warning, GPIO_MODE_OUTPUT);
    gpio_set_level(led_warning, 1);

    // SLEEP_MS(2000);

    // gpio_set_level(led_warning, 0);

    if (xTaskCreate(gpio_task, "gpio_task", 2048, this, 10, NULL) != pdPASS)
    {
        LOGE("Failed to create gpio_task ");
    }
#else
    Util::ExecuteCMD("echo 0 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio0/direction");

    Util::ExecuteCMD("echo 37 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio37/direction");

    Util::ExecuteCMD("echo 3 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio3/direction");

    Util::ExecuteCMD("echo 2 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio2/direction");

    Util::ExecuteCMD("echo 19 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio19/direction");

    Util::ExecuteCMD("echo 18 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio18/direction");

    Util::ExecuteCMD("echo 17 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio17/direction");

    Util::ExecuteCMD("echo 14 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio14/direction");

    Util::ExecuteCMD("echo 1 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio1/direction");

    Util::ExecuteCMD("echo 1 > /sys/class/gpio/gpio1/value");
#endif
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
#ifdef ESP_PLATFORM
    gpio_set_level(led_warning, stt);
#else
    string cmd1 = "echo " + to_string(!stt) + " > /sys/class/leds/linkit-smart-7688:orange:service/brightness";
    Util::ExecuteCMD(cmd1.c_str());
#endif
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