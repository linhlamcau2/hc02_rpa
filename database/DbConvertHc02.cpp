#ifdef ESP_PLATFORM
#include "Db.h"
#include "Log.h"
#include "Util.h"
#include "Base64.h"
#include "esp_littlefs.h"
#include <sys/stat.h>
#include "json.h"

sqlite3 *db_v1 = NULL;

int Db::OpenDbV1()
{
    if (IsHaveDb(DB_NAME_V1))
    {
        LOGD("open DB_NAME_V1");
        sqlite3_open_v2(DB_NAME_V1, &db_v1, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_JOURNAL, 0);
    }

    if (db)
    {
        if (db_v1)
        {
            return CODE_OK;
        }
        else
            LOGW("db_v1 not found");
    }
    else
        LOGW("db not found");

    return CODE_ERROR;
}

typedef void (*conversion_func)(sqlite3_stmt *stmt, int col_index, sqlite3_stmt *insert_stmt, int insert_index);

static void ConvertTypeFrimVerTableDevice(sqlite3_stmt *stmt, int col_index, sqlite3_stmt *insert_stmt, int insert_index)
{
    const char *version_str = (const char *)sqlite3_column_text(stmt, col_index);
    int major, minor;
    sscanf(version_str, "%d.%d", &major, &minor);
    int version_int = (major << 8) | minor;
    sqlite3_bind_int(insert_stmt, insert_index, version_int);
}

void copy_data_between_tables(sqlite3 *source_db, sqlite3 *dest_db, const char *source_table, const char *dest_table, const char **source_columns, const char **dest_columns, int num_columns, conversion_func *conversions)
{
    sqlite3_stmt *stmt;
    char sql_select[1024];
    char sql_insert[1024];
    char column_list[512] = "";
    char value_placeholders[512] = "";

    // Xây dựng chuỗi cột và giá trị
    for (int i = 0; i < num_columns; i++)
    {
        if (i > 0)
        {
            strcat(column_list, ", ");
            strcat(value_placeholders, ", ");
        }
        strcat(column_list, source_columns[i]);
        strcat(value_placeholders, "?");
    }

    puts(column_list);
    puts(value_placeholders);
    // Xây dựng câu lệnh SELECT
    snprintf(sql_select, sizeof(sql_select), "SELECT %s FROM %s", column_list, source_table);
    puts(sql_select);

    // Xây dựng câu lệnh INSERT
    snprintf(sql_insert, sizeof(sql_insert), "INSERT INTO %s (%s) VALUES (%s)", dest_table, column_list, value_placeholders);
    puts(sql_insert);

    // Chuẩn bị câu lệnh SELECT
    int rc = sqlite3_prepare_v2(source_db, sql_select, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        LOGE("Failed to prepare select statement: %s", sqlite3_errmsg(source_db));
        return;
    }

    // Chuẩn bị câu lệnh INSERT
    sqlite3_stmt *insert_stmt;
    rc = sqlite3_prepare_v2(dest_db, sql_insert, -1, &insert_stmt, NULL);
    if (rc != SQLITE_OK)
    {
        LOGE("Failed to prepare insert statement: %s", sqlite3_errmsg(dest_db));
        sqlite3_finalize(stmt);
        return;
    }

    // Thực thi câu lệnh SELECT và lấy các hàng
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        // Gắn giá trị vào câu lệnh INSERT
        for (int i = 0; i < num_columns; i++)
        {
            if (conversions != NULL && conversions[i] != NULL)
            {
                conversions[i](stmt, i, insert_stmt, i + 1);
            }
            else
            {
                if (sqlite3_column_type(stmt, i) == SQLITE_TEXT)
                {
                    sqlite3_bind_text(insert_stmt, i + 1, (const char *)sqlite3_column_text(stmt, i), -1, SQLITE_STATIC);
                }
                else if (sqlite3_column_type(stmt, i) == SQLITE_INTEGER)
                {
                    sqlite3_bind_int(insert_stmt, i + 1, sqlite3_column_int(stmt, i));
                }
                else if (sqlite3_column_type(stmt, i) == SQLITE_FLOAT)
                {
                    sqlite3_bind_double(insert_stmt, i + 1, sqlite3_column_double(stmt, i));
                }
                else if (sqlite3_column_type(stmt, i) == SQLITE_BLOB)
                {
                    sqlite3_bind_blob(insert_stmt, i + 1, sqlite3_column_blob(stmt, i), sqlite3_column_bytes(stmt, i), SQLITE_STATIC);
                }
                else
                {
                    sqlite3_bind_null(insert_stmt, i + 1);
                }
            }
        }

        // Thực thi câu lệnh INSERT
        rc = sqlite3_step(insert_stmt);
        if (rc != SQLITE_DONE)
        {
            LOGE("Failed to execute insert statement: %s", sqlite3_errmsg(dest_db));
        }

        // Đặt lại câu lệnh INSERT để sẵn sàng cho hàng tiếp theo
        sqlite3_reset(insert_stmt);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (rc != SQLITE_DONE && rc != SQLITE_OK)
    {
        LOGE("Failed to execute select statement: %s", sqlite3_errmsg(source_db));
    }

    // Hoàn tất câu lệnh
    sqlite3_finalize(stmt);
    sqlite3_finalize(insert_stmt);
}

int Db::ConvertTableDevice()
{
    const char *table = "Device";
    const char *columns[] = {"mac", "device_id", "name", "addr", "type", "firmware_version", "active_time", "update_time", "data"};
    int num_columns = 9;

    conversion_func conversions[] = {NULL, NULL, NULL, NULL, NULL, ConvertTypeFrimVerTableDevice, NULL, NULL, NULL};
    if (OpenDbV1() == CODE_OK)
    {
        copy_data_between_tables(db_v1, db, table, table, columns, columns, num_columns, conversions);
        return CODE_OK;
    }
    LOGW("Failed to open");
    return CODE_ERROR;
}

int Db::ConvertTableDeviceAttribute()
{
    // const char *table = "DeviceAttribute";
    // const char *columns[] = {"device_id", "attribute_id", "value"};
    // int num_columns = 3;
    // if (OpenDbV1() == CODE_OK)
    // {
    //     copy_data_between_tables(db_v1, db, table, table, columns, columns, num_columns, NULL);
    //     return CODE_OK;
    // }
    // LOGW("Failed to open");
    return CODE_ERROR;
}

int Db::ConvertTableDeviceBleChild()
{
    // Don't have data
    return CODE_OK;
}

int Db::ConvertTableDeviceInGroup()
{
    const char *table = "DeviceInGroup";
    const char *columns[] = {"group_id", "device_id", "element"};
    int num_columns = 3;
    if (OpenDbV1() == CODE_OK)
    {
        copy_data_between_tables(db_v1, db, table, table, columns, columns, num_columns, NULL);
        return CODE_OK;
    }
    LOGW("Failed to open");
    return CODE_ERROR;
}

int Db::ConvertTableDeviceInRoom()
{
    const char *table = "DeviceInRoom";
    const char *columns[] = {"room_id", "device_id"};
    int num_columns = 2;
    if (OpenDbV1() == CODE_OK)
    {
        copy_data_between_tables(db_v1, db, table, table, columns, columns, num_columns, NULL);
        return CODE_OK;
    }
    LOGW("Failed to open");
    return CODE_ERROR;
}

int Db::ConvertTableGateway()
{
    const char *table = "Gateway";
    const char *columns[] = {"mac", "gateway_id", "name", "version", "ble_netkey", "ble_appkey", "ble_devicekey", "ble_addr", "ble_iv_index", "dormitory", "refresh_token", "zigbee_netkey"};
    int num_columns = 12;
    if (OpenDbV1() == CODE_OK)
    {
        copy_data_between_tables(db_v1, db, table, table, columns, columns, num_columns, NULL);
        return CODE_OK;
    }
    LOGW("Failed to open");
    return CODE_ERROR;
}

int Db::ConvertTableGroup()
{
    const char *table = "[Group]";
    const char *columns[] = {"group_id", "group_addr", "name", "room_id"};
    int num_columns = 4;
    if (OpenDbV1() == CODE_OK)
    {
        copy_data_between_tables(db_v1, db, table, table, columns, columns, num_columns, NULL);
        return CODE_OK;
    }
    LOGW("Failed to open");
    return CODE_ERROR;
}

int Db::ConvertTableRoom()
{
    const char *table = "Room";
    const char *columns[] = {"room_id", "room_addr", "name"};
    int num_columns = 3;
    if (OpenDbV1() == CODE_OK)
    {
        copy_data_between_tables(db_v1, db, table, table, columns, columns, num_columns, NULL);
        return CODE_OK;
    }
    LOGW("Failed to open");
    return CODE_ERROR;
}

int Db::ConvertTableSceneBle()
{
    const char *table = "SceneBle";
    const char *columns[] = {"scene_ble_id", "scene_ble_addr", "name", "room_id", "is_favorite"};
    int num_columns = 5;
    if (OpenDbV1() == CODE_OK)
    {
        copy_data_between_tables(db_v1, db, table, table, columns, columns, num_columns, NULL);
        return CODE_OK;
    }
    LOGW("Failed to open");
    return CODE_ERROR;
}

static int TableRuleConvert(sqlite3_stmt *stmt, void *ptr)
{
    sqlite3 *db = (sqlite3 *)ptr;
    int s, index;
    if (stmt)
    {
        while (1)
        {
            s = sqlite3_step(stmt);
            if (s == SQLITE_ROW)
            {
                index = 0;

                string id = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
                string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
                int type = sqlite3_column_int(stmt, index++);
                bool enable = sqlite3_column_int(stmt, index++) ? true : false;
                uint16_t addr = sqlite3_column_int(stmt, index++);
                long create_at = sqlite3_column_int(stmt, index++);

                string ruledata;
                string decode = macaron::Base64::Decode(data, ruledata);
                if (decode == "")
                {
                    Json::Value ruleValue;
                    if (ruleValue.parse(ruledata) && ruleValue.isObject())
                    {
                        if (ruleValue.isMember("EVENT_TRIGGER_ID") && ruleValue["EVENT_TRIGGER_ID"].isString() &&
                            ruleValue.isMember("EACH_DAY") && ruleValue["EACH_DAY"].isArray() &&
                            ruleValue.isMember("LOGICAL_OPERATOR_ID") && ruleValue["LOGICAL_OPERATOR_ID"].isInt())
                        {
                            // process data rule
                            Json::Value ruleJson = Json::objectValue;

                            // id, enable, type, name
                            string name = "";
                            if (ruleValue.isMember("NAME") && ruleValue["NAME"].isString())
                                name = ruleValue["NAME"].asString();
                            ruleJson["id"] = ruleValue["EVENT_TRIGGER_ID"].asString();
                            ruleJson["name"] = name;
                            ruleJson["type"] = ruleValue["LOGICAL_OPERATOR_ID"].asInt();
                            ruleJson["enable"] = ruleValue["STATUS"].asInt();

                            // input
                            Json::Value ruleInput = Json::objectValue;

                            // input: repeat
                            Json::Value repeatDays = ruleValue["EACH_DAY"];
                            int mon = 0, tue = 0, wed = 0, thu = 0, fri = 0, sat = 0, sun = 0;
                            int repeat = 0;
                            if (repeatDays.size() > 0)
                            {
                                for (auto &rp : repeatDays)
                                {
                                    if (rp.isString())
                                    {
                                        if (rp == "EACHMONDAY")
                                            mon = 1;
                                        else if (rp == "EACHTUESDAY")
                                            tue = 1;
                                        else if (rp == "EACHWEDNESDAY")
                                            wed = 1;
                                        else if (rp == "EACHTHURSDAY")
                                            thu = 1;
                                        else if (rp == "EACHFRIDAY")
                                            fri = 1;
                                        else if (rp == "EACHSATURDAY")
                                            sat = 1;
                                        else if (rp == "EACHSUNDAY")
                                            sun = 1;
                                    }
                                }
                                repeat = sun * 64 + sat * 32 + fri * 16 + thu * 8 + wed * 4 + tue * 2 + mon;
                            }
                            ruleInput["repeat"] = repeat;

                            // input: timer (start, end)
                            string startAt = "";
                            string endAt = "";
                            if (ruleValue.isMember("START_AT") && ruleValue["START_AT"].isString())
                            {
                                startAt = ruleValue["START_AT"].asString();
                            }

                            if (ruleValue["END_AT"].isString() && ruleValue["END_AT"].isString())
                            {
                                endAt = ruleValue["END_AT"].asString();
                            }
                            if (startAt != "" || endAt != "")
                            {
                                Json::Value timerJson = Json::objectValue;
                                timerJson["start"] = startAt;
                                timerJson["end"] = endAt;
                                ruleInput["timer"] = timerJson;
                            }

                            // input: devices
                            Json::Value devInputArrayJson = Json::arrayValue;
                            if (ruleValue.isMember("INPUT_DEVICES") && ruleValue["INPUT_DEVICES"].isArray())
                            {
                                Json::Value listDevInput = ruleValue["INPUT_DEVICES"];

                                for (auto &devInput : listDevInput)
                                {
                                    if (devInput.isObject() && devInput.isMember("DEVICE_ID") && devInput["DEVICE_ID"].isString() && devInput.isMember("DEVICE_ATTRIBUTE") && devInput["DEVICE_ATTRIBUTE"].isObject())
                                    {
                                        Json::Value devInputJson = Json::objectValue;
                                        devInputJson["id"] = devInput["DEVICE_ID"].asString();
                                        Json::Value attributies = devInput["DEVICE_ATTRIBUTE"];
                                        if (attributies.isMember("ID") && attributies["ID"].isInt() &&
                                            attributies.isMember("VALUE") && (attributies["VALUE"].isInt() || attributies["VALUE"].isArray()))
                                        {
                                            {
                                                Json::Value dataJson = Json::objectValue;
                                                if (attributies["VALUE"].isInt())
                                                {
                                                    dataJson["op"] = "==";
                                                    dataJson[Device::BleAttributeIdToAttributeStr(attributies["ID"].asInt())] = attributies["VALUE"].asInt();
                                                }
                                                else if (attributies["VALUE"].isArray())
                                                {
                                                    Json::Value attr = attributies["VALUE"];
                                                    if (attr.size() == 2)
                                                    {
                                                        if (attr[0] == attr[1])
                                                        {
                                                            dataJson["op"] = "==";
                                                            dataJson[Device::BleAttributeIdToAttributeStr(attributies["ID"].asInt())] = attr[0];
                                                        }
                                                    }
                                                    else if (attr[0] != attr[1])
                                                    {
                                                        dataJson["op"] = "<>";
                                                        dataJson[Device::BleAttributeIdToAttributeStr(attributies["ID"].asInt())] = attr;
                                                    }
                                                }

                                                devInputJson["data"] = dataJson;
                                            }
                                        }
                                        devInputArrayJson.append(devInputJson);
                                    }
                                }
                            }
                            ruleInput["device"] = devInputArrayJson;
                            ruleJson["input"] = ruleInput;

                            // output
                            Json::Value ruleOutput = Json::arrayValue;

                            // output: device
                            if (ruleValue.isMember("OUTPUT_DEVICES") && ruleValue["OUTPUT_DEVICES"].isArray())
                            {
                                Json::Value listDevOutput = ruleValue["OUTPUT_DEVICES"];
                                for (auto &devOutput : listDevOutput)
                                {
                                    if (devOutput.isObject() && devOutput.isMember("DEVICE_ID") && devOutput["DEVICE_ID"].isString() &&
                                        devOutput.isMember("PROPERTIES") && devOutput["PROPERTIES"].isArray())
                                    {
                                        Json::Value ruleDevOutput = Json::objectValue;
                                        ruleDevOutput["deviceId"] = devOutput["DEVICE_ID"].asString();
                                        Json::Value dataDevOutput = Json::objectValue;
                                        Json::Value properties = devOutput["PROPERTIES"];
                                        for (auto &property : properties)
                                        {
                                            if (property.isObject() && property.isMember("ID") && property.isMember("VALUE") && property["ID"].isInt() && property["VALUE"].isInt())
                                            {
                                                dataDevOutput[Device::BleAttributeIdToAttributeStr(property["ID"].asInt())] = property["VALUE"].asInt();
                                            }
                                        }
                                        ruleDevOutput["data"] = dataDevOutput;
                                        ruleOutput.append(ruleDevOutput);
                                    }
                                }
                            }

                            // output: group
                            if (ruleValue.isMember("OUTPUT_GROUPS") && ruleValue["OUTPUT_GROUPS"].isArray())
                            {
                                Json::Value listGroupOutput = ruleValue["OUTPUT_GROUPS"];
                                for (auto &groupOutput : listGroupOutput)
                                {
                                    if (groupOutput.isObject() && groupOutput.isMember("GROUP_ID") && groupOutput["GROUP_ID"].isString() &&
                                        groupOutput.isMember("PROPERTIES") && groupOutput["PROPERTIES"].isArray())
                                    {
                                        Json::Value ruleGroupOutput = Json::objectValue;
                                        ruleGroupOutput["deviceId"] = groupOutput["GROUP_ID"].isString();
                                        Json::Value dataGroupOutput = Json::objectValue;
                                        Json::Value properties = groupOutput["PROPERTIES"];
                                        for (auto &property : properties)
                                        {
                                            if (property.isObject() && property.isMember("ID") && property.isMember("VALUE") && property["ID"].isInt() && property["VALUE"].isInt())
                                            {
                                                dataGroupOutput[Device::BleAttributeIdToAttributeStr(property["ID"].asInt())] = property["VALUE"].asInt();
                                            }
                                        }
                                        ruleGroupOutput["data"] = dataGroupOutput;
                                        ruleOutput.append(ruleGroupOutput);
                                    }
                                }
                            }

                            // outout:scene
                            if (ruleValue.isMember("OUTPUT_SCENES") && ruleValue["OUTPUT_SCENES"].isArray())
                            {
                                Json::Value listSceneOutput = ruleValue["OUTPUT_SCENES"];
                                for (auto &sceneOutput : listSceneOutput)
                                {
                                    if (sceneOutput.isObject() && sceneOutput.isMember("SCENE_ID") && sceneOutput["SCENE_ID"].isString())
                                    {
                                        Json::Value ruleSceneOutput = Json::objectValue;
                                        ruleSceneOutput["sceneId"] = sceneOutput["SCENE_ID"].asString();
                                        ruleOutput.append(ruleSceneOutput);
                                    }
                                }
                            }

                            ruleJson["output"] = ruleOutput;

                            // insert to new db
                            string ruleStr = ruleJson.toString();
                            ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
                            string sql = "INSERT OR REPLACE INTO Rule (rule_id, data, type, enable, rule_addr, create_at) VALUES ('" + id + "','" + macaron::Base64::Encode(ruleStr) + "'," + to_string(type) + ", " + to_string(enable) + ", " + to_string(addr) + "," + to_string(time(NULL)) + ");";
                            char *err_msg = 0;
                            int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
                            if (rc != SQLITE_OK)
                            {
                                LOGE("Error executing sql statement :%s", err_msg);
                                sqlite3_free(err_msg);
                            }
                        }
                    }
                    else
                    {
                        LOGW("RuleRead json format error rule: %s", ruledata.c_str());
                    }
                }
                else
                {
                    LOGW("Decode data err: %s", decode.c_str());
                }
            }
            else if (s == SQLITE_DONE)
            {
                return CODE_OK;
            }
            else
            {
                LOGE("RuleParse");
                return CODE_ERROR;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    return CODE_OK;
}

static int TableSceneDelayConvert(sqlite3_stmt *stmt, void *ptr)
{
    sqlite3 *db = (sqlite3 *)ptr;
    int s, index;
    if (stmt)
    {
        while (1)
        {
            s = sqlite3_step(stmt);
            if (s == SQLITE_ROW)
            {
                index = 0;
                string id = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
                int addr = sqlite3_column_int(stmt, index++);
                string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
                string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
                string sceneData;
                string decode = macaron::Base64::Decode(data, sceneData);
                if (decode == "")
                {
                    Json::Value sceneDelayValue;
                    if (sceneDelayValue.parse(sceneData) && sceneDelayValue.isObject())
                    {
                        if (sceneDelayValue.isMember("SCENE_ID") && sceneDelayValue["SCENE_ID"].isString())
                        {
                            Json::Value ruleJson = Json::objectValue;
                            int type = 0;
                            bool enable = true;
                            ruleJson["id"] = id;
                            ruleJson["name"] = name;
                            ruleJson["type"] = type;
                            ruleJson["enable"] = 1;
                            ruleJson["input"]["repeat"] = 255;

                            // output:
                            Json::Value outputJson = Json::arrayValue;
                            if (sceneDelayValue.isMember("DEVICES") && sceneDelayValue["DEVICES"].isArray())
                            {
                                Json::Value listDev = sceneDelayValue["DEVICES"];
                                for (auto &dev : listDev)
                                {
                                    if (dev.isObject() && dev.isMember("DELAY") && dev["DELAY"].isInt() &&
                                        dev.isMember("DEVICE_ID") && dev["DEVICE_ID"].isString() &&
                                        dev.isMember("PROPERTIES") && dev["PROPERTIES"].isArray())
                                    {
                                        Json::Value delayJson = Json::objectValue;
                                        delayJson["delay"] = dev["DELAY"].asInt();
                                        outputJson.append(delayJson);

                                        Json::Value devJson = Json::objectValue;
                                        devJson["deviceId"] = dev["DEVICE_ID"].asString();

                                        Json::Value dataJson = Json::objectValue;
                                        Json::Value properties = dev["PROPERTIES"];
                                        for (auto &property : properties)
                                        {
                                            if (property.isObject() && property.isMember("ID") && property.isMember("VALUE") && property["ID"].isInt() && property["VALUE"].isInt())
                                            {
                                                dataJson[Device::BleAttributeIdToAttributeStr(property["ID"].asInt())] = property["VALUE"].asInt();
                                            }
                                        }
                                        devJson["data"] = dataJson;
                                        outputJson.append(devJson);
                                    }
                                }
                            }
                            ruleJson["output"] = outputJson;

                            string ruleStr = ruleJson.toString();
                            ruleStr.erase(remove_if(ruleStr.begin(), ruleStr.end(), ::isspace), ruleStr.end());
                            string sql = "INSERT OR REPLACE INTO Rule (rule_id, data, type, enable, rule_addr, create_at) VALUES ('" + id + "','" + macaron::Base64::Encode(ruleStr) + "'," + to_string(type) + ", " + to_string(enable) + ", " + to_string(addr) + "," + to_string(time(NULL)) + ");";
                            // Sqlite_Exec(sql);
                            char *err_msg = 0;
                            int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
                            if (rc != SQLITE_OK)
                            {
                                LOGE("Error executing sql statement :%s", err_msg);
                                sqlite3_free(err_msg);
                            }
                        }
                        else
                        {
                            LOGW("Data error");
                        }
                    }
                    else
                    {
                        LOGW("SceneDelayRead json format error: %s", sceneData.c_str());
                    }
                }
                else
                {
                    LOGW("Decode data err: %s", decode.c_str());
                }
            }
            else if (s == SQLITE_DONE)
            {
                return CODE_OK;
            }
            else
            {
                return CODE_ERROR;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    return CODE_OK;
}

static int TableDeviceInSceneBleConvert(sqlite3_stmt *stmt, void *ptr)
{
    sqlite3 *db = (sqlite3 *)ptr;
    int s, index;
    if (stmt)
    {
        while (1)
        {
            s = sqlite3_step(stmt);
            if (s == SQLITE_ROW)
            {
                index = 0;
                string sceneBleId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
                string deviceId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
                string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));

                string devInSceneData;
                string decode = macaron::Base64::Decode(data, devInSceneData);
                if (decode == "")
                {
                    Json::Value devInSceneJson;
                    if (devInSceneJson.parse(devInSceneData) && devInSceneJson.isArray())
                    {
                        Json::Value dataSceneInsert = Json::objectValue;
                        for (auto &dataJson : devInSceneJson)
                        {
                            if (dataJson.isObject() && dataJson.isMember("ID") && dataJson.isMember("VALUE") && dataJson["ID"].isInt() && dataJson["VALUE"].isInt())
                            {
                                dataSceneInsert[Device::BleAttributeIdToAttributeStr(dataJson["ID"].asInt())] = dataJson["VALUE"].asInt();
                            }
                        }
                        string dataInsert = dataSceneInsert.toString();
                        dataInsert.erase(remove_if(dataInsert.begin(), dataInsert.end(), ::isspace), dataInsert.end());
                        string sql = "INSERT OR REPLACE INTO DeviceInSceneBle (scene_ble_id, device_id, data, create_at) VALUES ('" + sceneBleId + "','" + deviceId + "','" + macaron::Base64::Encode(dataInsert) + "', " + to_string(time(NULL)) + ");";
                        char *err_msg = 0;
                        int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
                        if (rc != SQLITE_OK)
                        {
                            LOGE("Error executing sql statement :%s", err_msg);
                            sqlite3_free(err_msg);
                        }
                    }
                    else
                    {
                        LOGE("data json is not object");
                    }
                    // TODO: Check cho du lieu V2
                }
                else
                {
                    LOGW("Decode data error");
                }
            }
            else if (s == SQLITE_DONE)
            {
                return CODE_OK;
            }
            else
            {
                LOGE("DeviceInGroupParse");
                return CODE_ERROR;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    return CODE_OK;
}

static int TableDeviceInGroupEdit(sqlite3_stmt *stmt, void *ptr)
{
    sqlite3 *db = (sqlite3 *)ptr;
    int s, index;
    if (stmt)
    {
        while (1)
        {
            s = sqlite3_step(stmt);
            if (s == SQLITE_ROW)
            {
                index = 0;
                string roomId = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
                uint16_t addr = sqlite3_column_int(stmt, index++);
                string name = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));
                long create_at = sqlite3_column_int(stmt, index++);
                string data = Util::setString(reinterpret_cast<const char *>(sqlite3_column_text(stmt, index++)));

                string sql = "DELETE FROM DeviceInGroup WHERE group_id= '" + roomId + "';";
                char *err_msg = 0;
                int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
                if (rc != SQLITE_OK)
                {
                    LOGE("Error executing sql statement :%s", err_msg);
                    sqlite3_free(err_msg);
                }
            }
            else if (s == SQLITE_DONE)
            {
                return CODE_OK;
            }
            else
            {
                LOGE("RoomParse");
                return CODE_ERROR;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    return CODE_OK;
}

int Db::ReadAll_V1(string table, void *listPtr, int (*Parse)(sqlite3_stmt *, void *))
{
    int rc = SQLITE_ERROR;
    sqlite3_stmt *stmt;
    string sql = "SELECT * FROM " + table + ";";

    if (!Parse)
    {
        LOGW("Parse func NULL");
        return CODE_ERROR;
    }

    LOGD("ReadAll table %s", table.c_str());
    rc = sqlite3_prepare_v2(db_v1, sql.c_str(), sql.length(), &stmt, NULL);
    if (rc == SQLITE_OK)
    {
        LOGD("sqlite3_prepare_v2 successfully");
        Parse(stmt, db);
        sqlite3_finalize(stmt);
    }
    else
    {
        LOGW("SQL error: %d - %s", rc, sqlite3_errmsg(db_v1));
    }

    return rc;
}

int Db::ConvertTableDeviceInSceneBle()
{
    if (OpenDbV1() == CODE_OK)
    {
        ReadAll_V1("DeviceInSceneBle", NULL, TableDeviceInSceneBleConvert);
        return CODE_OK;
    }
    LOGW("Failed to open");
    return CODE_ERROR;
}

int Db::ConvertTableRule()
{
    if (OpenDbV1() == CODE_OK)
    {
        ReadAll_V1("Rule", NULL, TableRuleConvert);
        return CODE_OK;
    }
    LOGW("Failed to open");
    return CODE_ERROR;
}

int Db::ConvertTableSceneDelay()
{
    if (OpenDbV1() == CODE_OK)
    {
        ReadAll_V1("SceneDelay", NULL, TableSceneDelayConvert);
        return CODE_OK;
    }
    LOGW("Failed to open");
    return CODE_ERROR;
}

int Db::EditTableDeviceInGroup()
{
    if (OpenDbV1() == CODE_OK)
    {
        ReadAll_V1("Room", NULL, TableDeviceInGroupEdit);
        return CODE_OK;
    }
    LOGW("Failed to open");
    return CODE_ERROR;
}

#endif