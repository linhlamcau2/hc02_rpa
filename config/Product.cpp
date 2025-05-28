#include"Product.h"

static ProductMap product_map = {
        {"00051018", {CTCU_BLE_CN_O4T,4,"22032"}},
        {"00059765", {CTCU_BLE_CN_O4T,4,"22032"}},

        {"00051010", {CTCU_BLE_CN_O3T,3,"22030"}},
        {"00051009", {CTCU_BLE_CN_O2T,2,"22028"}},
        {"00051008", {CTCU_BLE_CN_O1T,1,"22026"}},


        {"00072392", {CTCU_WF_CN_04T_2W_SP,4,"22043"}},
        {"00065845", {CTCU_WF_CN_04T_2W_SP,4,"22043"}},
        
        {"00072391", {CTCU_WF_CN_03T_2W_SP,3,"22042"}},
        {"00065844", {CTCU_WF_CN_03T_2W_SP,3,"22042"}},

        {"00072389", {CTCU_WF_CN_01T_2W_SP,1,"22040"}},
        {"00065842", {CTCU_WF_CN_01T_2W_SP,1,"22040"}},

        {"00086573", {DOWNLOAD_AT58_90_10W,1,"22040"}},
        {"000865734", {DOWNLOAD_AT58_110_12W,1,"22040"}},

    };

bool is_product_exist(const string& prod_code, ProductInfo &prod) {
    if(product_map.find(prod_code) != product_map.end()) {
        prod = product_map[prod_code];
        return true;
    }
    return false;
}


