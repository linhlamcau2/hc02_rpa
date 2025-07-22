#include"Product.h"

static ProductMap product_map = {
//CTCU CN BLE
        {"00051040", {CTCU_BLE_CN_O4T,4,"22032"}},
        // {"00051040", {CTCU_BLE_CN_O4T_MN,4,"22032"}},

        {"00051039", {CTCU_BLE_CN_O3T,3,"22030"}},
        // {"00051039", {CTCU_BLE_CN_O3T_MN,3,"22030"}},
        
        {"00051038", {CTCU_BLE_CN_O2T,2,"22028"}},
        // {"00051038", {CTCU_BLE_CN_O2T_MN,2,"22028"}},

        {"00051037", {CTCU_BLE_CN_O1T,1,"22026"}},
        // {"00051037", {CTCU_BLE_CN_O1T_MN,1,"22026"}},

        // {"00051019", {CTCU_BLE_CN_REMT,1,"0"}},
        // {"00059766", {CTCU_BLE_CN_REMT_MN,1,"0"}},
//CTCU V BLE
        {"00051045", {CTCU_BLE_V_O4T,4,"22033"}},
        // {"00051045", {CTCU_BLE_V_O4T_MN,4,"22033"}},

        {"00051044", {CTCU_BLE_V_O3T,3,"22031"}},
        // {"00051044", {CTCU_BLE_V_O3T_MN,3,"22031"}},

        {"00051043", {CTCU_BLE_V_O2T,2,"22029"}},
        // {"00051043", {CTCU_BLE_V_O2T_MN,2,"22029"}},

        {"00051042", {CTCU_BLE_V_O4T,1,"22027"}},
        // {"00051042", {CTCU_BLE_V_O4T_MN,1,"22027"}},

        // {"00051024", {CTCU_BLE_V_REMT,1,"0"}},
        // {"00059761", {CTCU_BLE_V_REMT_MN,1,"0"}},
//CTCU CN WF
        {"00065850", {CTCU_WF_CN_04T_2W_SP_MN,4,"22043"}},
        // {"00065850", {CTCU_WF_CN_04T_2W_SP,4,"22043"}},
        
        {"00065849", {CTCU_WF_CN_03T_2W_SP_MN,3,"22042"}},
        // {"00065849", {CTCU_WF_CN_03T_2W_SP,3,"22042"}},

        {"00065848", {CTCU_WF_CN_02T_2W_SP_MN,2,"22041"}},
        // {"00065848", {CTCU_WF_CN_02T_2W_SP,2,"22041"}},

        {"00065847", {CTCU_WF_CN_01T_2W_SP_MN,1,"22040"}},
        // {"00065847", {CTCU_WF_CN_01T_2W_SP,1,"22040"}},

//CTCU V WF
        {"00070782", {CTCU_WF_V_04T_2W_SP,4,"22051"}},
        // {"00070782", {CTCU_WF_V_04T_2W_SP_MN,4,"22051"}},

        {"00070781", {CTCU_WF_V_03T_2W_SP,3,"22050"}},
        // {"00070781", {CTCU_WF_V_03T_2W_SP_MN,3,"22050"}},

        {"00070780", {CTCU_WF_V_02T_2W_SP,2,"22049"}},
        // {"00070780", {CTCU_WF_V_02T_2W_SP_MN,2,"22049"}},

        {"00070779", {CTCU_WF_V_01T_2W_SP,1,"22048"}},
        // {"00070779", {CTCU_WF_V_01T_2W_SP_MN,1,"22048"}},
//DOWNLIGHT AT58
        {"00086573", {DOWNLIGHT_AT58_90_10W,1,"22040"}},
        {"00086562", {DOWNLIGHT_AT58_90_10W,1,"22040"}},

        {"00086574", {DOWNLIGHT_AT58_110_12W,1,"22040"}},
        {"00086563", {DOWNLIGHT_AT58_110_12W,1,"22040"}},
// CTR BLE
        {"00051041", {CTR_BLE_CN,3,"22034"}},
        // {"00051041", {CTR_BLE_CN_MN,3,"22034"}},
        {"00051046", {CTR_BLE_V,3,"22035"}},
        // {"00051046", {CTR_BLE_V_MN,3,"22035"}},
// CTR BLE WIFI
        {"00065985", {CTR_BLE_WF_CN,3,"22046"}},
        // {"00065985", {CTR_BLE_WF_CN_MN,3,"22046"}},
        {"00070784", {CTR_BLE_WF_V,3,"22047"}},
        // {"00070784", {CTR_BLE_WF_V_MN,3,"22047"}},
//CTCC BLE
        {"00052727", {CTCC_BLE_CN,3,"22018"}},
        {"00064137", {CTCC_BLE_V,3,"22025"}},
//CTCC BLE WIFI
        {"00060875", {CTCC_BLE_WF_CN,3,"22044"}},
        {"00071108", {CTCC_BLE_WF_V,3,"22045"}},
    };

bool is_product_exist(const string& prod_code, ProductInfo &prod) {
    if(product_map.find(prod_code) != product_map.end()) {
        prod = product_map[prod_code];
        return true;
    }
    return false;
}


