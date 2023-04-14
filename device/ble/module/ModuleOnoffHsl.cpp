#include "ModuleOnoffHsl.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleOnoffHsl::ModuleOnoffHsl(Device *device, uint32_t addr) : Module(device, addr)
{
    onoff = 0;
    h = 0;
    s = 0;
    l = 0;

    idOnoff = BLE_ATTRIBUTE_ONOFF;
    idH = BLE_ATTRIBUTE_HUE;
    idS = BLE_ATTRIBUTE_SATURATION;
    idL = BLE_ATTRIBUTE_LUMINANCE;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleOnoffHsl::InitAttribute(int id, double value)
{
    if (this->idOnoff == id)
        onoff = value;
    else if (this->idH == id)
        h = value;
    else if (this->idS == id)
        s = value;
    else if (this->idL == id)
        l = value;
}

void ModuleOnoffHsl::SaveAttribute()
{
    database->DeviceAttributeAddOrReplace(device, idOnoff, onoff);
    database->DeviceAttributeAddOrReplace(device, idH, h);
    database->DeviceAttributeAddOrReplace(device, idS, s);
    database->DeviceAttributeAddOrReplace(device, idL, l);
}
#endif

// int ModuleOnoffHsl::InputData(Json::Value &dataValue, Json::Value &jsonValue)
// {
//     if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt() && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
//     {
//         int id = dataValue["ID"].asInt();
//         if (this->idOnoff == id || this->idH == id || this->idS == id || this->idL == id)
//         {
//             if (this->idOnoff == id)
//                 onoff = dataValue["VALUE"].asInt();
//             else if (this->idH == id)
//                 h = dataValue["VALUE"].asInt();
//             else if (this->idS == id)
//                 s = dataValue["VALUE"].asInt();
//             else if (this->idL == id)
//                 l = dataValue["VALUE"].asInt();
//             BuildTelemetryValue(jsonValue);
//             return CODE_OK;
//         }
//     }
//     return CODE_ERROR;
// }

int ModuleOnoffHsl::InputData(uint8_t *data, int len, Json::Value &jsonValue, Json::Value &jsonValueV2)
{
    typedef struct __attribute__((packed))
    {
        uint16_t opcode;
        uint8_t header;
        uint8_t status_mode;
        uint16_t l;
        uint16_t h;
        uint16_t s;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == BLE_MESH_OPCODE_UPDATE)
    {
        onoff = (data_message->status_mode >> 4) & 0x0f;
        if ((data_message->status_mode & 0x0F) == 0)
        {
            l = data_message->l;
            h = data_message->h;
            s = data_message->s;
        }
#ifdef CONFIG_SAVE_ATTRIBUTE
        SaveAttribute();
#endif
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

void ModuleOnoffHsl::BuildTelemetryValue(Json::Value &jsonValue)
{
    Json::Value dataValue;
    dataValue["ID"] = idOnoff;
    dataValue["VALUE"] = onoff;
    jsonValue.append(dataValue);
    dataValue["ID"] = idH;
    dataValue["VALUE"] = h;
    jsonValue.append(dataValue);
    dataValue["ID"] = idS;
    dataValue["VALUE"] = s;
    jsonValue.append(dataValue);
    dataValue["ID"] = idL;
    dataValue["VALUE"] = l;
    jsonValue.append(dataValue);
}

void ModuleOnoffHsl::BuildTelemetryValueV2(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_ONOFF] = onoff;
    jsonValue[KEY_ATTRIBUTE_HUE] = h;
    jsonValue[BLE_ATTRIBUTE_SATURATION] = s;
    jsonValue[BLE_ATTRIBUTE_LUMINANCE] = l;
}
