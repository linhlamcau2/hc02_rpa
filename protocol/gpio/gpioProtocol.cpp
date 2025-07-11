#include "gpioProtocol.h"
#include "Util.h"
#include "Log.h"
#include "Gateway.h"
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <map>
#include "BleProtocol.h"
#include "Config.h"

#define COUNT_PRESS 5
#define COUNT_HOLD 50

enum
{
    STEP_1_POWER_ON_PAIR,
    STEP_2_PRESS_1_1,
    STEP_3_PRESS_1_2,
    STEP_4_PRESS_1_3,
    STEP_5_PRESS_2_1,
    STEP_6_PRESS_2_2,
    STEP_7_PRESS_2_3,
    STEP_8_PRESS_3_1,
    STEP_9_PRESS_3_2,
    STEP_10_PRESS_3_3,
    STEP_11_DIM_0,
    STEP_12_POWER_OFF_SHORT,
    STEP_13_POWER_CYCLE,
    STEP_14_POWER_6S,
    STEP_15_DELETE_PAIRING,

    STEP_DONE
};

enum
{
    BUTTON_NULL = 0,
    BUTTON_PRESS,
    BUTTON_HOLD,
};

typedef void (*ButtonHandler_t)(int mode);

typedef struct
{
    int stt;
    int button_id;
    int count_stt;
    ButtonHandler_t handler;
} ButtonMap_t;

int pin_pow_k9b = 14;

int led_success = 17;

int button_start = 37;
int button_pause = 12;

int k9b_at58_but1 = 0;
int k9b_at58_but2 = 2;
int k9b_at58_but3 = 3;

int k9b_at58_but[3] = {0, 2, 3};

GPIOProtocol *gpioProtocol = NULL;

std::mutex mtx;
std::condition_variable cv;

bool running = false;
bool paused = false;
bool reset_requested = false;

static void led_running_wr(int stt)
{
    std::string cmd = "echo " +to_string(stt) + " > /sys/class/gpio/gpio" + to_string(led_success) + "/value";
    Util::ExecuteCMD(cmd.c_str());
}

static void led_success_wr(int stt) 
{
    std::string cmd = "echo " + to_string(!stt) + " > /sys/class/leds/linkit-smart-7688:orange:service/brightness"; // chan 18 //vang
    Util::ExecuteCMD(cmd.c_str());
}

static void led_pause_wr(int stt)
{
    std::string cmd = "echo "+ to_string(!stt)+" > /sys/class/leds/linkit-smart-7688:orange:internet/brightness"; //chan 19  // do
    Util::ExecuteCMD(cmd.c_str());
}

void rpa_running_display()
{
    led_running_wr(1);
    led_pause_wr(0);
}
void rpa_pause_display()
{
    led_pause_wr(1);
}
void rpa_stop_display()
{
    led_running_wr(0);
    led_pause_wr(0);
    led_success_wr(0);
}
static void gpio_supply_power_k9b()
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
    std::string cmd = "echo 1 > /sys/class/gpio/gpio" + to_string(pin_pow_k9b) + "/value";
    Util::ExecuteCMD(cmd.c_str());
    cmd = "echo 1 > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness"; // chan 15
    Util::ExecuteCMD(cmd.c_str());
    cmd = "echo 0 > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness"; // chan 16
    Util::ExecuteCMD(cmd.c_str());

    SLEEP_MS(70);
    cmd = "echo 1 > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness";
    Util::ExecuteCMD(cmd.c_str());

    SLEEP_MS(200);
    cmd = "echo 0 > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness";
    Util::ExecuteCMD(cmd.c_str());

    SLEEP_MS(70);
    cmd = "echo 1 > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness";
    Util::ExecuteCMD(cmd.c_str());

    SLEEP_MS(1000);
    cmd = "echo 0 > /sys/class/gpio/gpio" + to_string(pin_pow_k9b) + "/value";
    Util::ExecuteCMD(cmd.c_str());
#endif
}

void set_but_k9b(int id, int stt)
{
    for (int i = 0; i < 3; i++) 
    {
        if (id & (1 << i)) 
        {
            // LOGI("Button %d pressed %d", i,stt);
            std::string cmd = "echo "+to_string(stt) +" > /sys/class/gpio/gpio" + to_string(k9b_at58_but[i]) + "/value";
            Util::ExecuteCMD(cmd.c_str());
        }
    }
}

void GPIOProtocol::k9b_press(int id_but)
{
    set_but_k9b(id_but, 0);
    SLEEP_MS(20);
    gpio_supply_power_k9b();
    SLEEP_MS(20);
    set_but_k9b(id_but, 1);
    // LOGI("Button %d pressed", id_but);
}

void at58_power_on()
{
    LOGI("at58_power_on");
    vector<int> addr_ctcu = config->GetCtcudAddr();
    for(int i = 0; i < addr_ctcu.size(); i++)
    {
        bleProtocol->ControlRelayOfSwitch(addr_ctcu[i], 4,0xff, 1);
    }
}

void at58_power_off()
{
    LOGI("at58_power_off");
    vector<int> addr_ctcu = config->GetCtcudAddr();
    for(int i = 0; i < addr_ctcu.size(); i++)
    {
        bleProtocol->ControlRelayOfSwitch(addr_ctcu[i], 4,0xff, 0);
    }
}

void at58_pair_k9b()
{
    for (int i = 0; i < 6; i++)
    {
        // nhan nut 2+3
        gpioProtocol->k9b_press(0b110);
        SLEEP_MS(1200);
    }
}

void at58_del_k9b()
{
    for (int i = 0; i < 6; i++)
    {
        // nhan nut 1+2
        gpioProtocol->k9b_press(0b011);
        SLEEP_MS(1200);
    }
}

void at58_handle_repeat(int id_but)
{
    gpioProtocol->k9b_press(1<< id_but);
    SLEEP_MS(3000);
}

void at58_reset_power()
{
    for (int i = 0; i < 5; i++)
    {
        // bat den
        at58_power_on();
        SLEEP_MS(3500);
        at58_power_off();
        SLEEP_MS(3000);
    }
}

void execute_state(int state)
{
    LOGI("Staging  %d", state);
    switch (state)
    {
    case STEP_1_POWER_ON_PAIR:

        // Cấp nguồn cho đèn. Nhấn nút 2 và 3 mỗi nút 5 lần để ghép nối
        at58_power_off();
        SLEEP_MS(3000);
        at58_power_on();
        SLEEP_MS(2500);
        at58_pair_k9b();
        break;

    case STEP_2_PRESS_1_1:
    case STEP_3_PRESS_1_2:
    case STEP_4_PRESS_1_3:
        // Nhấn nút 1: đèn chuyển sang màu vàng
        at58_handle_repeat(0);
        break;

    case STEP_5_PRESS_2_1:
    case STEP_6_PRESS_2_2:
    case STEP_7_PRESS_2_3:

        // Nhấn nút 2: đèn chuyển sang màu trung tính
        at58_handle_repeat(1);
        break;

    case STEP_8_PRESS_3_1:
    case STEP_9_PRESS_3_2:
    case STEP_10_PRESS_3_3:
        // Nhấn nút 3: đèn chuyển sang màu trắng
        at58_handle_repeat(2);
        break;

    case STEP_11_DIM_0:
        // Nhấn đồng th ời nút 1 + 3: DIM 0%
        gpioProtocol->k9b_press(0b101);
        SLEEP_MS(2000);
        break;

    case STEP_12_POWER_OFF_SHORT:
        //Ngắt nguồn 0.5s
        at58_power_off();
        SLEEP_MS(3000);
        break;

    case STEP_13_POWER_CYCLE:
        // Cấp nguồn 4s - đèn sáng trắng 100%
        at58_reset_power();
        break;

    case STEP_14_POWER_6S:
        // Cấp nguồn 6s - đèn sáng trắng 100%%
        at58_power_on();
        SLEEP_MS(7000);
        at58_power_off();
        SLEEP_MS(3000);
        break;

    case STEP_15_DELETE_PAIRING:
        //Cấp nguồn lại. Nhấn nút 1 + 2 năm laanf dee xoa ghep noi
        at58_power_on();
        SLEEP_MS(4000);
        at58_del_k9b();
        break;

    case STEP_DONE:
        break;
    default:
        break;
    }
}

void GPIOProtocol ::process_rpa()
{
    int current_step = STEP_1_POWER_ON_PAIR;

    while (1)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []{ return running; });

            if (reset_requested)
            {
                current_step = STEP_1_POWER_ON_PAIR;
                reset_requested = false;
                continue;
            }
        }

        execute_state(current_step);

        {
            std::lock_guard<std::mutex> lock(mtx);
            if (paused)
            {
                at58_power_off();
                running = false;               
            }
            if (reset_requested)
                continue;
        }

        current_step++;

        if (current_step >= STEP_DONE)
        {
            current_step = STEP_1_POWER_ON_PAIR;
            running = false;
            rpa_stop_display();
            continue;
        }
    }
}

void button_start_handler(int mode)
{
    if (mode == BUTTON_PRESS)
    {
        {
            LOGI("but start press");
            std::lock_guard<std::mutex> lock(mtx);
            if(paused) 
            {
                at58_power_on();
                paused = false;
                SLEEP_MS(1000);
            }
            running = true;
            rpa_running_display();
        }
        
        cv.notify_one();
    }
    else if (mode == BUTTON_HOLD && running)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            reset_requested = true;
            paused = false;
        }
        cv.notify_one();
    }
}

void button_pause_handler(int mode)
{
    if (mode == BUTTON_PRESS)
    {
        std::lock_guard<std::mutex> lock(mtx);
        paused = true;
        rpa_pause_display();
    }
}

int read_button_value(int id_but)
{
    std::string cmd = "cat /sys/class/gpio/gpio" + std::to_string(id_but) + "/value";
    std::string value = Util::ExecuteCMD(cmd.c_str());
    return std::stoi(value);
}

static ButtonMap_t ButMap[] = {
    {1,button_start, 0, button_start_handler},
    {1,button_pause, 0, button_pause_handler},
};

int detect_button(int index)
{
    int stt = read_button_value(ButMap[index].button_id);
    if (ButMap[index].stt!=stt)
    {
        ButMap[index].count_stt++;
        if (ButMap[index].count_stt == COUNT_PRESS)
        {
            ButMap[index].stt = stt;
            ButMap[index].count_stt = 0;
            return BUTTON_PRESS; // Button pressed
        }
    }
    else
    {
        ButMap[index].count_stt = 0; // Reset count after handling press
    }
    return BUTTON_NULL;
}

void GPIOProtocol :: button_thread_func()
{
    while (1)
    {
        for (int i = 0; i < sizeof(ButMap) / sizeof(ButMap[0]); i++)
        {
            int stt = detect_button(i);
            if (stt == BUTTON_PRESS || stt == BUTTON_HOLD)
            {
                ButMap[i].handler(stt);
            }
        }
    }
}


void GPIOProtocol ::gpio_init()
{
    // k9b
    Util::ExecuteCMD("echo 14 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio14/direction");

    Util::ExecuteCMD("echo 0 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio0/direction");

    Util::ExecuteCMD("echo 2 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio2/direction");

    Util::ExecuteCMD("echo 3 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio3/direction");

    // button handle proc
    Util::ExecuteCMD("echo 12 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio12/direction");

    Util::ExecuteCMD("echo 37 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio37/direction");

    // led
    Util::ExecuteCMD("echo 17 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio17/direction");


    set_but_k9b(1, 1);
    set_but_k9b(2, 1);
    set_but_k9b(4, 1);
    rpa_stop_display();

    ButMap[0].stt=read_button_value(ButMap[0].button_id);
    ButMap[1].stt=read_button_value(ButMap[1].button_id);

    thread proc_rpa(bind(&GPIOProtocol ::process_rpa, this));
    proc_rpa.detach();

    thread button_func(bind(&GPIOProtocol :: button_thread_func, this));
    button_func.detach();
}