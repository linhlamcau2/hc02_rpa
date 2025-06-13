#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <signal.h>
#include <fstream>
#include "json.h"
#include "Log.h"
// #include <CUnit/CUnit.h>
// #include <CUnit/Basic.h>

#define TAG "MAIN"

using namespace std;

// int add(int a, int b) {
//     return a + b;
// }

// int sub(int a, int b) {
//     return a - b;
// }

// void test_add() {
//     CU_ASSERT(add(2, 3) == 5);
//     CU_ASSERT(add(-1, -1) == -2);
// }

// void test_sub() {
//     CU_ASSERT(sub(5, 3) == 2);
//     CU_ASSERT(sub(2, 3) == -1);
// }

// int main() {
//     CU_initialize_registry();

//     CU_pSuite suite = CU_add_suite("MathSuite", 0, 0);
//     CU_add_test(suite, "test_add", test_add);
//     CU_add_test(suite, "test_sub", test_sub);

//     CU_basic_run_tests();  // In kết quả ra console
//     CU_cleanup_registry();
//     return 0;
// }
int main(int argc, char *argv[])
{
    while(1)
    {
        usleep(5000);
        LOGI("Start smh test");
    }
	
	return 0;
}
