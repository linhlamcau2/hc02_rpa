Thiết bị gửi lên server, server phản hồi:
- v1/json/req/<mac>/server -> v1/json/resp/server/<mac>

Server gửi xuống thiết bị, thiết bị phản hồi:
- v1/json/req/server/<mac> -> v1/json/resp/<mac>/server

*note:
- req, resp: loại bản tin
- <mac>/server, server/<mac>: chiều đi của bản tin

# Bản tin MQTT

## I. Bản tin điều khiển
### 1. Bản tin thiết bị báo trạng thái: HC->server
Request:

```json
{
    "cmd": "deviceUpdate",
    "rqi": "abc123456",
    "data": {
        "device": [{
                "id": "b717f8d8-6f18-43c0-ae46-69c32998f653",
                "data": {
                    "stt": 1,
                    "bt0": 1,
                    "onoff0": 0,
                    "h": 0,
                    "s": 2,
                    "l": 4,
                    "dim": 50
                }
            },
            {
                "id": "b717f8d8-6f18-43c0-ae46-69c32998f654",
                "data": {
                    "stt": 1,
                    "temp": 25,
                    "hum": 90
                }
            }
        ]
    }
}
```

Response:
```json
{
    "cmd": "deviceUpdateRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

* Note:
```
- Liệt kê tất cả các tham số có thể có của các loại thiết bị theo tên rút gọn, bao gồm thiết bị nhiều element và các chức năng phụ khác (màu led rgb). Ví dụ:
    + bt0: giá trị nút ấn đầu tiên của công tắc 4 nút.
    + rBt0: giá trị màu R của nút bấm đầu tiên của bảng cảnh.
    + temp: nhiệt độ.
    + pin: mức pin.
- deviceId có thể chứa các thông tin text bất kỳ hay ko: Ví dụ tạo bằng cách: <mac>_<thời gian tạo>
```
### 2. Bản tin điều khiển: server->HC
Request:
```json
{
    "cmd": "controlDev",
    "rqi": "abc123456",
    "data": {
        "id": "b717f8d8-6f18-43c0-ae46-69c32998f653",
        "data": {
            "bt0": 1,
            "h": 0,
            "s": 2,
            "l": 4,
            "dim": 50
        }
    }
}
```

Response:
```json
{
    "cmd": "controlDevRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

* Note:
```
- Sau khi điều khiển xong, thiết bị phản hồi trạng thái mới, HC sẽ gửi thêm bản tin báo trạng thái (bản tin số 1)
```

### 3. Điều khiển tất cả thiết bị: server->HC
Request:
```json
{
    "cmd": "controlAllDev",
    "rqi": "abc123456",
    "data": {
        "bt0": 1,
        "h": 0,
        "s": 2,
        "l": 4,
        "dim": 50
    }
}

```

Response:
```json
{
    "cmd": "controlAllDevRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

### 4. Điều khiển Group: server->HC
Request:
```json
{
    "cmd": "controlGroup",
    "rqi": "abc123456",
    "data": {
        "id": "b717f8d8-6f18-43c0-ae46-69c32998f653",
        "data": {
            "bt0": 1,
            "h": 0,
            "s": 2,
            "l": 4,
            "dim": 50
        }
    }
}
```

Response:
```json
{
    "cmd": "controlGroupRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

* Note:
```
- Sau khi điều khiển xong, thiết bị phản hồi trạng thái mới, HC sẽ gửi thêm bản tin báo trạng thái (bản tin số 1)
```

### 5. Kích hoạt Scene: server->HC
Request:
```json
{
    "cmd": "controlScene",
    "rqi": "abc123456",
    "data": {
        "id": "b717f8d8-6f18-43c0-ae46-69c32998f653"
    }
}
```

Response:
```json
{
    "cmd": "controlSceneRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

* Note:
```
- Sau khi điều khiển xong, thiết bị phản hồi trạng thái mới, HC sẽ gửi thêm bản tin báo trạng thái (bản tin số 1)
```

### 6. Bản tin device update: app->HC
Request:
```json
{
    "cmd": "requestDevStt",
    "rqi": "abc123456",
    "data": {
        "time": "1451649600512" 
    }
}
```

Response:
```json
{
    "cmd": "requestDevSttRsp",
    "rqi": "abc123456",
    "data": {
        "device": [{
                "id": "b717f8d8-6f18-43c0-ae46-69c32998f653",
                "data": {
                    "stt": 1,
                    "bt0": 1,
                    "onoff0": 0,
                    "h": 0,
                    "s": 2,
                    "l": 4,
                    "dim": 50
                }
            },
            {
                "id": "b717f8d8-6f18-43c0-ae46-69c32998f654",
                "data": {
                    "stt": 1,
                    "temp": 25,
                    "hum": 90
                }
            }
        ]
    }
}
```

* Note:
```
- Chỉ dùng cho giao tiếp local
```



## II. Bản tin cấu hình
### II.1. Device
### II.1.1. Quét thiết bị: server->HC
Request:
```json
{
    "cmd": "startScanBle",
    "rqi": "abc123456",
    "data": {}
}
```

Response:
```json
{
    "cmd": "startScanBleRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

### II.1.2. Dừng quét thiết bị: server->HC
Request:
```json
{
    "cmd": "stopScanBle",
    "rqi": "abc123456",
    "data": {}
}
```

Response:
```json
{
    "cmd": "stopScanBleRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

### II.1.3. Báo thiết bị mới: HC->server
Request:
```json
{
    "cmd": "newDev",
    "rqi": "abc123456",
    "data": {
        "device": [{
                "id": "b717f8d8-6f18-43c0-ae46-69c32998f653",
                "addr": 2,
                "type": 22014,
                "mac": "AB:DE:EF",
                "ver": "1.0.2",
                "devKey": "b717f8d8-6f18-43c0-ae46-69c32998f653",
                "netKey": "b717f8d8-6f18-43c0-ae46-69c32998f653",
                "appKey": "b717f8d8-6f18-43c0-ae46-69c32998f653"
            }
        ]
    }
}
```

Response:
```json
{
    "cmd": "newDevRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

* Note:
```
- Với cả thiết bị nhiều element cũng chỉ gửi 1 bản tin như các thiết bị bình thường
```

### II.1.4. Xóa thiết bị khỏi HC: server->HC
Request:
```json
{
    "cmd": "delDev",
    "rqi": "abc123456",
    "data": {
        "devices": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653",
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

Response:
```json
{
    "cmd": "delDevRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "success": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653"
        ],
        "failed": [
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

### II.2. Group
### II.2.1. Tạo nhóm: server->HC
Request:
```json
{
    "cmd": "createGroup",
    "rqi": "abc123456",
    "data": {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "name": "abc",
        "devices": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653",
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

Response:
```json
{
    "cmd": "createGroupRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "addr": 49152,
        "success": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653"
        ],
        "failed": [
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

### II.2.2. Thêm thiết bị vào nhóm: server->HC
Request:
```json
{
    "cmd": "addDevToGroup",
    "rqi": "abc123456",
    "data": {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "device": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653",
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

Response:
```json
{
    "cmd": "addDevToGroupRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "success": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653"
        ],
        "failed": [
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

### II.2.3. Xóa thiết bị khỏi nhóm: server->HC
Request:
```json
{
    "cmd": "delDevFromGroup",
    "rqi": "abc123456",
    "data": {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "device": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653",
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

Response:
```json
{
    "cmd": "delDevFromGroupRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "success": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653"
        ],
        "failed": [
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

### II.2.4. Xóa nhóm: server->HC
Request:
```json
{
    "cmd": "delGroup",
    "rqi": "abc123456",
    "data": {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2"
    }
}
```

Response:
```json
{
    "cmd": "delGroupRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "success": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653"
        ],
        "failed": [
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

### II.3. Scene
### II.3.1. Tạo/sửa cảnh: server->HC
Request:
```json
{
    "cmd": "createScene",//editScene
    "rqi": "abc123456",
    "data": {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "name": "abc",
        "devices": [{
                "id": ["b717f8d8-6f18-43c0-ae46-69c32998f653"],
                "data": {
                    "bt0": 1,
                    "h": 0,
                    "s": 2,
                    "l": 4,
                    "dim": 50
                }, {
                "id": ["b717f8d8-6f18-43c0-ae46-69c32998f654"],
                "data": {
                    "bt0": 1,
                    "h": 0,
                    "s": 2,
                    "l": 4,
                    "dim": 50
                }
            }
        ],
        "groups": [{//có dùng với group được không?
                "id": ["b717f8d8-6f18-43c0-ae46-69c32998f655"],
                "data": {
                    "bt0": 1,
                    "h": 0,
                    "s": 2,
                    "l": 4,
                    "dim": 50
                }
            }
        ]
    }
}
```

Response:
```json
{
    "cmd": "createSceneRsp",//editSceneRsp
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "addr": 1,
        "success": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653"
        ],
        "failed": [
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

### II.3.2. Xóa cảnh: server->HC
Request:
```json
{
    "cmd": "delScene",
    "rqi": "abc123456",
    "data": {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2"
    }
}
```

Response:
```json
{
    "cmd": "delSceneRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "success": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653"
        ],
        "failed": [
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

### II.3.3. Goi cảnh: server->HC
Request:
```json
{
    "cmd": "callScene",
    "rqi": "abc123456",
    "data": {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2"
    }
}
```

Response:
```json
{
    "cmd": "callSceneRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

### II.4. Control Scene
### II.4.1. Tạo kịch bản gọi cảnh: server->HC
Khi thiết bị đáp ứng yêu cầu kịch bản thì thực hiện gọi cảnh tương ứng, áp dụng cho cả controller, sensor và screen

Request:
```json
{
    "cmd": "createSceneController",//editSceneController
    "rqi": "abc123456",
    "data": {
        "devId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "data": [{
                "lux": [200,600],
                "op": "<>"
            }, {
                "pir": 1,
                "op": "=="
            }
        ]
        "sceneId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2"
    }
}

```

Response:
```json
{
    "cmd": "createSceneControllerRsp",//editSceneControllerRsp
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

### II.4.2. Xóa kịch bản gọi cảnh: server->HC
Request:
```json
{
    "cmd": "delSceneController",
    "rqi": "abc123456",
    "data": {
        "devId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "data": [{
                "lux": [200,600],
                "op": "<>"
            }, {
                "pir": 1,
                "op": "=="
            }
        ]
    }
}
```

Response:
```json
{
    "cmd": "delSceneControllerRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

* Note:
```
- Khi gửi lệnh xóa, không có trường 'data' thì coi như xóa cả thiết bị
```

### II.5. Rule
### II.5.1. Tạo kịch bản: server->HC
Request:
```json
{
    "cmd": "createRule",//editRule
    "rqi": "abc123456",
    "data": {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "repeat": 0,//có hiệu lực vào các ngày trong tuần. bit 0->6 cho 7 ngày trong tuần
        "time": {//có hiệu lực vào thời gian trong ngày (không dùng thì để null)
            "start": "00:11",
            "end": "11:23"
        },
        "type": 1,//cơ chế kiểm tra input: and, or
        "input": {
            "timer": {//hẹn giờ
                "repeat": 255,//có hiệu lực vào các ngày trong tuần. bit 0->6 cho 7 ngày trong tuần
                "time": "00:11"
            },
            "device": [{
                    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
                    "data": {
                        "op": ">",
                        "temp": 25
                    }
                }, {
                    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
                    "data": {
                        "op": "<=",
                        "co2": 25
                    }
                }
            ]
        },
        "output": [{//thực hiện theo thứ tự
                "devId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",//điều khiển thiết bị
                "data": {
                    "onoff": 1//bật đèn/công tắc
                }
            }, {
                "time": 100//tạm dừng 100s
            }, {
                "sceneId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2"//kích hoạt cảnh
            }, {
                "time": 30//tạm dừng 30s
            }, {
				"groupId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",//điều khiển nhóm
                "data": {
                    "onoff": 1
                }
            }
        ]
    }
}


```

Response:
```json
{
    "cmd": "createRuleRsp",//editRuleRsp
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

### II.5.2. Xóa kịch bản: server->HC
Request:
```json
{
    "cmd": "delRule",
    "rqi": "abc123456",
    "data": {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2"
    }
}
```

Response:
```json
{
    "cmd": "delRuleRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

### II.6. Room
### II.6.1. Tạo phòng: server->HC
Request:
```json
{
  "cmd": "createRoom",
  "rqi": "abc123456",
  "data": {
    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
    "name": "Phòng khách",
    "devices": [
      "b717f8d8-6f18-43c0-ae46-69c32998f653",
      "b717f8d8-6f18-43c0-ae46-69c32998f654"
    ],
    "scenes": [
      "b717f8d8-6f18-43c0-ae46-69c32998f650", //6 kịch bản mặc định
      "b717f8d8-6f18-43c0-ae46-69c32998f651",
      "b717f8d8-6f18-43c0-ae46-69c32998f652",
      "b717f8d8-6f18-43c0-ae46-69c32998f653",
      "b717f8d8-6f18-43c0-ae46-69c32998f654",
      "b717f8d8-6f18-43c0-ae46-69c32998f655"
    ]
  }
}
```

Response:
```json
{
    "cmd": "createRoomRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "success": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653"
        ],
        "failed": [
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

### II.6.2. Thêm thiết bị vào phòng: server->HC
Request:
```json
{
  "cmd": "addDevToRoom",
  "rqi": "abc123456",
  "data": {
    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
    "devices": [
      "b717f8d8-6f18-43c0-ae46-69c32998f653",
      "b717f8d8-6f18-43c0-ae46-69c32998f654"
    ]
  }
}
```

Response:
```json
{
    "cmd": "addDevToRoomRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "success": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653"
        ],
        "failed": [
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

### II.6.3. Xóa thiết bị khỏi phòng: server->HC
Request:
```json
{
  "cmd": "delDevFromRoom",
  "rqi": "abc123456",
  "data": {
    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
    "devices": [
      "b717f8d8-6f18-43c0-ae46-69c32998f653",
      "b717f8d8-6f18-43c0-ae46-69c32998f654"
    ]
  }
}
```

Response:
```json
{
    "cmd": "delDevFromRoomRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "success": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653"
        ],
        "failed": [
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

### II.6.4. Xóa phòng: server->HC
Request:
```json
{
  "cmd": "delRoom",
  "rqi": "abc123456",
  "data": {
    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2"
  }
}
```

Response:
```json
{
    "cmd": "delRoomRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "success": [
            "b717f8d8-6f18-43c0-ae46-69c32998f653"
        ],
        "failed": [
            "b717f8d8-6f18-43c0-ae46-69c32998f654"
        ]
    }
}
```

### II.7. Cấu hình HC
### II.1.1. Reset HC: server->HC
Request:
```json
{
  "cmd": "resetHc",
  "rqi": "abc123456",
  "data": {}
}
```

Response:
```json
{
    "cmd": "resetHcRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```
