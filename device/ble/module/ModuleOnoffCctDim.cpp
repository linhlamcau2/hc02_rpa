#include "ModuleOnoffCctDim.h"
#include "Log.h"
#include "Util.h"
#include "BleDefine.h"
#include "Device.h"
#include "BleProtocol.h"
#include "Db.h"

ModuleOnoffCctDim::ModuleOnoffCctDim(Device *device, uint32_t addr) : Module(device, addr)
{
    onoff = 0;
    dim = 0;
    cct = 0;

    idOnoff = BLE_ATTRIBUTE_ONOFF;
    idCct = BLE_ATTRIBUTE_CCT;
    idDim = BLE_ATTRIBUTE_DIM;
}

#ifdef CONFIG_SAVE_ATTRIBUTE
void ModuleOnoffCctDim::InitAttribute(int id, double value)
{
    if (this->idOnoff == id)
        onoff = value;
    else if (this->idCct == id)
        cct = value;
    else if (this->idDim == id)
        dim = value;
}

void ModuleOnoffCctDim::SaveAttribute()
{
    database->DeviceAttributeAddOrReplace(device, idOnoff, onoff);
    database->DeviceAttributeAddOrReplace(device, idCct, cct);
    database->DeviceAttributeAddOrReplace(device, idDim, dim);
}
#endif

// int ModuleOnoffCctDim::InputData(Json::Value &dataValue, Json::Value &jsonValue)
// {
//     if (dataValue.isObject() && dataValue.isMember("ID") && dataValue["ID"].isInt() && dataValue.isMember("VALUE") && dataValue["VALUE"].isInt())
//     {
//         int id = dataValue["ID"].asInt();
//         if (this->idOnoff == id || this->idCct == id || this->idDim == id)
//         {
//             if (this->idOnoff == id)
//                 onoff = dataValue["VALUE"].asInt();
//             else if (this->idCct == id)
//                 cct = dataValue["VALUE"].asInt();
//             else if (this->idDim == id)
//                 dim = dataValue["VALUE"].asInt();
//             BuildTelemetryValue(jsonValue);
//             return CODE_OK;
//         }
//     }
//     return CODE_ERROR;
// }

int ModuleOnoffCctDim::InputData(uint8_t *data, int len, Json::Value &jsonValue)
{
    typedef struct __attribute__((packed))
    {
        uint16_t opcode;
        uint8_t header;
        uint8_t status_mode;
        uint16_t dim;
        uint16_t cct;
    } data_message_t;
    data_message_t *data_message = (data_message_t *)data;
    if (data_message->opcode == BLE_MESH_OPCODE_UPDATE)
    {
        if (data_message->header == 0x02)
        {
            onoff = (data_message->status_mode >> 4) & 0x0F;
            if ((data_message->status_mode & 0x0f) == 1)
            {
                dim = data_message->dim;
                cct = data_message->cct;
            }
        }
#ifdef CONFIG_SAVE_ATTRIBUTE
        SaveAttribute();
#endif
        BuildTelemetryValue(jsonValue);
        return CODE_OK;
    }
    return CODE_ERROR;
}

void ModuleOnoffCctDim::BuildTelemetryValue(Json::Value &jsonValue)
{
    Json::Value dataValue;
    dataValue["ID"] = idOnoff;
    dataValue["VALUE"] = onoff;
    jsonValue.append(dataValue);
    dataValue["ID"] = idCct;
    dataValue["VALUE"] = ((cct - 800) / 192);
    jsonValue.append(dataValue);
    dataValue["ID"] = idDim;
    dataValue["VALUE"] = (dim * 100) / 65535;
    jsonValue.append(dataValue);
}

void ModuleOnoffCctDim::BuildTelemetryValueV2(Json::Value &jsonValue)
{
    jsonValue[KEY_ATTRIBUTE_ONOFF] = onoff;
    jsonValue[KEY_ATTRIBUTE_DIM] = dim;
    jsonValue[KEY_ATTRIBUTE_CCT] = cct;
}
