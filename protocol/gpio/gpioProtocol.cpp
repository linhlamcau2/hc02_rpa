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

#define COUNT_PRESS 5
#define COUNT_HOLD 50

int pin_pow_k9b = 14;

int led_success = 17;

int button_start = 2;
int button_pause = 46;

int k9b_at58_but1 = 0;
int k9b_at58_but2 = 37;
int k9b_at58_but3 = 3;

int k9b_at58_but[3] = {0, 37, 3};

GPIOProtocol *gpioProtocol = NULL;

std::mutex mtx;
std::condition_variable cv;

bool running = true;
bool paused = false;
bool reset_requested = false;

std::atomic<bool> stop_program{false};

int GPIOProtocol ::gpio_get(int index)
{
    if (index < 0 || index > num)
    {
        return -1;
    }
    return state_gpio[index];
}

void GPIOProtocol ::gpio_init()
{
    // k9b
    Util::ExecuteCMD("echo 14 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio14/direction");

    Util::ExecuteCMD("echo 0 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio0/direction");

    Util::ExecuteCMD("echo 37 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio37/direction");

    Util::ExecuteCMD("echo 3 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio3/direction");

    // button handle proc
    Util::ExecuteCMD("echo 2 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio2/direction");

    Util::ExecuteCMD("echo 46 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo in > /sys/class/gpio/gpio46/direction");

    // led
    Util::ExecuteCMD("echo 17 > /sys/class/gpio/export");
    Util::ExecuteCMD("echo out > /sys/class/gpio/gpio17/direction");

    thread testSwitchThread(bind(&GPIOProtocol::detect_button, this));
    testSwitchThread.detach();
}

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
    STEP_16_PRESS_1_FINAL,

    STEP_DONE
};

void k9b_press(int id_but)
{
    // Simulate pressing the button with id_but
    // This is a placeholder for actual button press logic
    LOGI("Button %d pressed", id_but);
}

void at58_power_on()
{
    //     SLEEP_MS(2000);
}

void at58_power_off()
{
    // SLEEP_MS(2000);
}

void at58_pair_k9b()
{
    for (int i = 0; i < 5; i++)
    {
        // nhan nut 2+3
        SLEEP_MS(2000);
    }
}

void at58_del_k9b()
{
    for (int i = 0; i < 5; i++)
    {
        // nhan nut 1+2
        SLEEP_MS(2000);
    }
}

void at58_handle_repeat(int id_but)
{
    for (int i = 0; i < 3; i++)
    {
        // nhan nut id_but
        SLEEP_MS(5000);
    }
}

void at58_reset_power()
{
    for (int i = 0; i < 4; i++)
    {
        // bat den
        at58_power_on();
        SLEEP_MS(3000);
        at58_power_off();
        SLEEP_MS(1000);
    }
}

void execute_state(int state)
{
    switch (state)
    {
    case STEP_1_POWER_ON_PAIR:

        // printf("Cấp nguồn cho đèn. Nhấn nút 2 và 3 mỗi nút 5 lần để ghép nối.\n");
        at58_power_on();
        SLEEP_MS(2000);
        at58_pair_k9b();
        break;

    case STEP_2_PRESS_1_1:
    case STEP_3_PRESS_1_2:
    case STEP_4_PRESS_1_3:
        // printf("Nhấn nút 1: đèn chuyển sang màu vàng.\n");
        at58_handle_repeat(1);
        break;

    case STEP_5_PRESS_2_1:
    case STEP_6_PRESS_2_2:
    case STEP_7_PRESS_2_3:

        // printf("Nhấn nút 2: đèn chuyển sang màu trung tính.\n");
        at58_handle_repeat(2);
        break;

    case STEP_8_PRESS_3_1:
    case STEP_9_PRESS_3_2:
    case STEP_10_PRESS_3_3:
        // printf("Nhấn nút 3: đèn chuyển sang màu trắng.\n");
        at58_handle_repeat(3);
        break;

    case STEP_11_DIM_0:
        // printf("Nhấn đồng th ời nút 1 + 3: DIM 0%%.\n");
        k9b_press(0b101);
        SLEEP_MS(2000);
        break;

    case STEP_12_POWER_OFF_SHORT:
        // printf("Ngắt nguồn 0.5s.\n");
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
        at58_power_off();
        SLEEP_MS(1000);
        break;

    case STEP_13_POWER_CYCLE:
        // printf("Cấp nguồn 4s - đèn sáng trắng 100%%.\n");
        at58_reset_power();
        break;

    case STEP_14_POWER_6S:
        // printf("Cấp nguồn 6s - đèn sáng trắng 100%%.\n");
        at58_power_on();
        SLEEP_MS(6000);
        at58_power_off();
        SLEEP_MS(1000);
        break;

    case STEP_15_DELETE_PAIRING:
        // printf("Cấp nguồn lại. Nhấn nút 1 + 2 năm lần để xóa ghép nối.\n");
        at58_power_on();
        SLEEP_MS(2000);
        at58_del_k9b();
        break;

    case STEP_16_PRESS_1_FINAL:
        // printf("Nhấn nút 1: đèn chuyển sang màu trắng 100%%.\n");
        k9b_press(0b100);
        SLEEP_MS(2000);
        break;

    case STEP_DONE:
        // printf("Hoàn tất quy trình.\n");
        break;
    default:
        // printf("Trạng thái không hợp lệ.\n");
        break;
    }
}

void logic_thread_func()
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
                running = false;
            }
            if (reset_requested)
                continue;
        }

        current_step++;

        if (current_step >= STEP_DONE)
        {
            running = false;
            break;
        }
    }
}

enum
{
    BUTTON_NULL = 0,
    BUTTON_PRESS,
    BUTTON_HOLD,
};

typedef void (*ButtonHandler_t)(int mode);

typedef struct
{
    int button_id;
    int count_stt;
    ButtonHandler_t handler;
} ButtonMap_t;

void button_start_handler(int mode)
{
    if (mode == BUTTON_PRESS)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            paused = false;
            running = true;
        }
        cv.notify_one();
    }
    else if (mode == BUTTON_HOLD)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            reset_requested = true;
            running = true;
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
    }
}

int read_button_value(int id_but)
{
    std::string cmd = "cat /sys/class/gpio/gpio" + std::to_string(id_but) + "/value";
    std::string value = Util::ExecuteCMD(cmd.c_str());
    return std::stoi(value);
}

static ButtonMap_t ButMap[] = {
    {button_start, 0, button_start_handler},
    {button_pause, 0, button_pause_handler},
};

int detect_button(int index)
{
    if (read_button_value(ButMap[index].button_id))
    {
        ButMap[index].count_stt++;
        if (ButMap[index].count_stt == COUNT_PRESS)
        {
            return BUTTON_PRESS; // Button pressed
        }
        else if (ButMap[index].count_stt == COUNT_HOLD)
        {
            return BUTTON_HOLD; // Button held
        }
        else if (ButMap[index].count_stt > COUNT_HOLD)
        {
            ButMap[index].count_stt = COUNT_HOLD + 1;
            return BUTTON_NULL; // Button not pressed or held
        }
    }
    else
    {
        ButMap[index].count_stt = 0; // Reset count after handling press
    }
    return BUTTON_NULL;
    // for(const auto& pair : ButMap)
    // {

    // }
}

void button_thread_func()
{
    int stt_but_start = 0;
    int stt_but_pause = 0;
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

void GPIOProtocol ::process_rpa()
{

    SLEEP_MS(10000);
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
    cmd = "echo 1 > /sys/class/leds/linkit-smart-7688:orange:ble1/brightness"; // chan 15
    Util::ExecuteCMD(cmd.c_str());
    cmd = "echo 0 > /sys/class/leds/linkit-smart-7688:orange:ble2/brightness"; // chan 16
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
    string cmd1 = "echo " + to_string(!stt) + " > /sys/class/leds/linkit-smart-7688:orange:service/brightness"; // chan 18
    Util::ExecuteCMD(cmd1.c_str());
#endif
}

void GPIOProtocol ::reset_led_in_proc()
{
#ifdef ESP_PLATFORM
    gpio_set_level(led_success, 0);
    gpio_set_level(led_fail, 0);
#else
    string cmd = "echo 1 > /sys/class/leds/linkit-smart-7688:orange:internet/brightness"; // chan 19
    string cmd1 = "echo 0 > /sys/class/gpio/gpio" + to_string(led_success) + "/value";
    Util::ExecuteCMD(cmd.c_str());
    Util::ExecuteCMD(cmd1.c_str());
#endif
}

