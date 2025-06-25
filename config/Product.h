#ifndef PRODUCT_H
#define PRODUCT_H


using namespace std;
#include <string>
#include <map>

enum
{
    CTCU_BLE_CN_O4T = 0,
    CTCU_BLE_CN_O3T,
    CTCU_BLE_CN_O2T,
    CTCU_BLE_CN_O1T,

    CTCU_BLE_CN_REMT,

    CTCU_BLE_CN_O4T_MN,
    CTCU_BLE_CN_O3T_MN,
    CTCU_BLE_CN_O2T_MN,
    CTCU_BLE_CN_O1T_MN,
    CTCU_BLE_CN_REMT_MN,

    CTCU_BLE_V_O4T,
    CTCU_BLE_V_O3T,
    CTCU_BLE_V_O2T,
    CTCU_BLE_V_O1T,
    CTCU_BLE_V_REMT,

    CTCU_BLE_V_O4T_MN,
    CTCU_BLE_V_O3T_MN,
    CTCU_BLE_V_O2T_MN,
    CTCU_BLE_V_O1T_MN,
    CTCU_BLE_V_REMT_MN,

    CTCU_WF_CN_01T_2W_SP,
    CTCU_WF_CN_02T_2W_SP,
    CTCU_WF_CN_03T_2W_SP,
    CTCU_WF_CN_04T_2W_SP,

    CTCU_WF_CN_01T_2W_SP_MN,
    CTCU_WF_CN_02T_2W_SP_MN,
    CTCU_WF_CN_03T_2W_SP_MN,
    CTCU_WF_CN_04T_2W_SP_MN,

    CTCU_WF_V_01T_2W_SP,
    CTCU_WF_V_02T_2W_SP,
    CTCU_WF_V_03T_2W_SP,
    CTCU_WF_V_04T_2W_SP,

    CTCU_WF_V_01T_2W_SP_MN,
    CTCU_WF_V_02T_2W_SP_MN,
    CTCU_WF_V_03T_2W_SP_MN,
    CTCU_WF_V_04T_2W_SP_MN,

    DOWNLIGHT_AT58_90_10W,
    DOWNLIGHT_AT58_110_12W,

    CTR_BLE_CN,
    CTR_BLE_CN_MN,
    CTR_BLE_V,
    CTR_BLE_V_MN,

    CTR_BLE_WF_CN,
    CTR_BLE_WF_CN_MN,
    CTR_BLE_WF_V,
    CTR_BLE_WF_V_MN,

    CTCC_BLE_CN,
    CTCC_BLE_V,

    CTCC_BLE_WF_CN,
    CTCC_BLE_WF_V
};

typedef struct
{
    int type; 
    int num_ele;
    string dev_type;  
} ProductInfo;

typedef map<string, ProductInfo> ProductMap;

bool is_product_exist(const string& prod_code, ProductInfo &prod);
#endif