#include "gpioProtocol.h"
#include "driver/gpio.h"
#include "util.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Log.h"
#include "Gateway.h"
gpio_num_t gpio_arr[] = {GPIO_NUM_27, GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_14};
gpio_num_t  pin_pow_k9b = GPIO_NUM_25;
gpio_num_t  pin_down_k9b = GPIO_NUM_26;
gpio_num_t  pin_up_k9b = GPIO_NUM_33;

gpio_num_t  led_success = GPIO_NUM_32;
gpio_num_t  led_fail = GPIO_NUM_18;
gpio_num_t  led_warning = GPIO_NUM_19;

GPIOProtocol *gpioProtocol = NULL;

// GPIOProtocol :: GPIOProtocol()
// {
//     gpio_init();
// }



int GPIOProtocol :: gpio_get(int index)
{
    if (index < 0 || index > num)
    {
        return -1;
    }
    return state_gpio[index];
}

int GPIOProtocol :: gpio_read(int index)
{
    if (index < 0 || index > num)
    {
        return -1;
    }
    return gpio_get_level(gpio_arr[index]);
}

int GPIOProtocol :: dectect_gpio(int index)
{
    int stt = !(this ->gpio_read(index));
    if (stt != state_gpio[index])
    {
        this -> count[index] ++;
        if (this -> count[index] >= CYCLE_ACTIVE)
        {
            this -> count[index] = 0;
            state_gpio[index] = stt;
            LOGI("GPIO %d: %d", index, stt);
            // if(gateway != NULL)
            //     gateway->CloudPublish("relay: " + to_string(index+1) + " "+ to_string(stt));
            return 1;
        }
    }
    return 0;
}

int GPIOProtocol :: handle_gpio(int index)
{
    return 0;
}

static void gpio_task(void * pvParameters)
{
    GPIOProtocol *gpioProtocol = (GPIOProtocol *)pvParameters;
    gpioProtocol->on_gpio();
}

void GPIOProtocol :: on_gpio()
{
    while (1)
    {
        for (int i = 0; i < num; i++)
        {
            if (dectect_gpio(i))
            {
                handle_gpio(i);
            }
        }
        // gpio_supply_power_k9b();
        // LOGI("gpio_supply_power_k9b");
        SLEEP_MS(5);
        // gpioProtocol->gpio_supply_power_k9b();
        // SLEEP_MS(2000);
    }
    vTaskDelete(NULL);
}
void GPIOProtocol :: gpio_init()
{
    num = sizeof(gpio_arr) / sizeof(gpio_arr[0]);
    state_gpio.resize(num,0);
    count.resize(num,0);
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

    gpio_set_direction(led_success, GPIO_MODE_OUTPUT);    //led_warning
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
}

void GPIOProtocol :: gpio_supply_power_k9b()
{
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
}

void GPIOProtocol :: set_led_success()
{
    gpio_set_level(led_success, 1);
    gpio_set_level(led_fail, 0);
}

void GPIOProtocol :: set_led_fail()
{
    gpio_set_level(led_success, 0);
    gpio_set_level(led_fail, 1);
}

void GPIOProtocol :: set_led_warning(uint8_t stt)
{
    // gpio_set_level(led_success, 1);
    // gpio_set_level(led_fail, 1);
    gpio_set_level(led_warning, stt);
}

void GPIOProtocol :: reset_led_in_proc()
{
    gpio_set_level(led_success, 0);
    gpio_set_level(led_fail, 0);
    // gpio_set_level(led_warning, 0);
}

void GPIOProtocol :: set_mode_input()
{
    for (int i = 0; i < num; i++)
    {
        gpio_reset_pin(gpio_arr[i]);
        gpio_set_direction(gpio_arr[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(gpio_arr[i], GPIO_PULLUP_ONLY); 
    }
}