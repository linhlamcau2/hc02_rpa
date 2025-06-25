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

        {"00086573", {DOWNLIGHT_AT58_90_10W,1,"22040"}},
        {"000865734", {DOWNLIGHT_AT58_110_12W,1,"22040"}},
// CTR BLE
        {"00051019", {CTR_BLE_CN,3,"22034"}},
        {"00059766", {CTR_BLE_CN_MN,3,"22034"}},
        {"00051024", {CTR_BLE_V,3,"22035"}},
        {"00059761", {CTR_BLE_V_MN,3,"22035"}},
// CTR BLE WIFI
        {"00065846", {CTR_BLE_WF_CN,3,"22046"}},
        {"00072393", {CTR_BLE_WF_CN_MN,3,"22046"}},
        {"00071106", {CTR_BLE_WF_V,3,"22047"}},
        {"00072398", {CTR_BLE_WF_V_MN,3,"22047"}},
//CTCC BLE
        {"00052726", {CTCC_BLE_CN,3,"22018"}},
        {"00064138", {CTCC_BLE_V,3,"22025"}},
//CTCC BLE WIFI
        {"00073169", {CTCC_BLE_WF_CN,3,"22044"}},
        {"00071107", {CTCC_BLE_WF_V,3,"22045"}},
    };

bool is_product_exist(const string& prod_code, ProductInfo &prod) {
    if(product_map.find(prod_code) != product_map.end()) {
        prod = product_map[prod_code];
        return true;
    }
    return false;
}


