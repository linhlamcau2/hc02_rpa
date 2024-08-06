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
                    "bt": 1,
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
    + bt: giá trị nút ấn đầu tiên của công tắc 4 nút.
    + rBt: giá trị màu R của nút bấm đầu tiên của bảng cảnh.
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
            "bt": 1,
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
        "bt": 1,
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
    "cmd": "controlDevRsp",
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
            "bt": 1,
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
```
- HC phản hồi bản tin số 1
```

* Note:
```
- Chỉ dùng cho giao tiếp local
```
### 7. Bản tin kích hoạt rule
Request:
```json
{
  "cmd": "actionRule",
  "rqi": "abc123456",
  "data": {
    "id": "16bd1c0d-e5b3-8333-8978-c2cc5b445a7b"
  }
}
```
Response:
```json
{
    "cmd": "actionRuleRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
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
                "data":{
                    "devKey": "b717f8d8-6f18-43c0-ae46-69c32998f653",
                    "ipLan": "192.168.1.1"
                }
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
### II.1.3.1. Update name device
Request:
```json
{
  "cmd": "updateDeviceName",
  "rqi": "mobile-eaa07ce2-e924-4594-bb68-c66c81e91b85",
  "data": {
    "id": "440e90f7-612c-933a-b0b4-e975d3b44d83",
    "name": "Cảm biến nhiệt/ẩm 001"
  }
}
```
```json
{
  "cmd": "updateDeviceNameRsp",
  "rqi": "mobile-eaa07ce2-e924-4594-bb68-c66c81e91b85",
  "data": {
    "code": 0
  }
}
```
Response:
### II.1.4. Xóa thiết bị khỏi nhóm: server->HC
Request:
```json
{
    "cmd": "delDev",
    "rqi": "abc123456",
    "data": {
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
        ],
        "roomId": "670bf986-0605-4709-850c-225475d8950b"
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
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
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
    "cmd": "addDevToGroupRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
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
    "cmd": "delDevFromGroupRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
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
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
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
        "roomId": "670bf986-0605-4709-850c-225475d8950b",
        "devices": [
            {
            "id": "b717f8d8-6f18-43c0-ae46-69c32998f653",
            "data": {
                "bt": 1,
                "h": 0,
                "s": 2,
                "l": 4,
                "dim": 50
                }
            },
            {
            "id": "b717f8d8-6f18-43c0-ae46-69c32998f654",
            "data": {
                "bt": 1,
                "h": 0,
                "s": 2,
                "l": 4,
                "dim": 50
                }
            }
        ],
        "groups": [{//có dùng với group được không?
                "id": "b717f8d8-6f18-43c0-ae46-69c32998f655",
                "data": {
                    "bt": 1,
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

### II.4. Control Scene
#### II.4.1. Tạo kịch bản gọi cảnh: server->HC
Khi thiết bị đáp ứng yêu cầu kịch bản thì thực hiện gọi cảnh tương ứng, áp dụng cho cả controller, sensor và screen

Request:
```json
{
  "cmd": "createSceneController",
  "rqi": "abc123456",
  "data": {
    "devId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
    "data": [
      {
        "properties": [
          {
            "bt": 2,
            "op": "=="
          },
          {
            "pir": 1,
            "op": "=="
          }
        ],
        "scene": {
          "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
          "icon": 1,
          "name": "mùa hè"
        }
      }
    ],
    "roomId": "abcd1234"
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
#### II.4.2. Xóa kịch bản gọi cảnh: server->HC
Request:
```json
{
  "cmd": "delSceneController",
  "rqi": "abc123456",
  "data": {
    "devId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
    "data": [
      {
        "properties": [
          {
            "bt": 2,
            "op": "=="
          },
          {
            "pir": 1,
            "op": "=="
          }
        ],
        "scene": {
          "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
          "icon": 1,
          "name": "mùa hè"
        }
      }
    ],
    "roomId": "abcd1234"
  }
}
```

Response:
```json
{
    "cmd": "delSceneControllerRsp",//editSceneControllerRsp
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```

### II.5 Kịch bản
### II.5.2. Xóa kịch bản: server->HC
Request:
```json
{
  "cmd": "createRule",
  "rqi": "abc123456",
  "data": {
    "roomId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
    "name": "abc",
    "type": -2,
    "enable": 1,
    "input": {
      "repeat": 255,
      "timer": {
        "start": "00:11",
        "end":"00:12"
      },
      "device": [
        {
          "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
          "data": {
            "op": ">",
            "temp": 25
          }
        },
        {
          "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
          "data": {
            "op": "<=",
            "co2": 25
          }
        }
      ]
    },
    "output": [
      {
        "deviceId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "data": {
          "onoff": 1,
          "dim": 20
        }
      },
      {
        "delay": 3
      },
      {
        "groupId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "data": {
          "onoff": 1,
          "dim": 20
        }
      },
      {
        "delay": 8
      },
      {
        "sceneId": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2"
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
		"groups":[
			{
				"id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
				"name": "Đèn LED Downlight",
				"type": 22001
			}
		],
		"scenes":[
			{
				"id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
				"name": "Mùa hè",
				"groups": [
					{
						"id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
						"data": {
							"dim": 100
						}
					}
				]
			},
      {
				"id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
				"name": "Mùa Đông",
				"groups": [
					{
						"id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
						"data": {
							"dim": 100
						}
					}
				]
			},
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
  "rqi": "mobile-70429560-2239-46d0-8845-1390db74cb1d",
  "data": {
    "id": "a625def5-d0b5-4d68-87ca-e91ce3ae6a28",
    "name":"phòng mới",
    "devices": [
      "935723f9-ad31-5432-9633-30443baf73f6",
      "c232bfa0-670c-b43c-9d66-fbee7322fd1d",
      "123e0a94-a3c5-c73f-90ce-f9249ce88bb9"
    ],
    "groups": [
      {
        "id": "2c184914-7f4f-467d-af88-59f66ea2e5b4",
        "name": "Downlight SMT",
        "type": 12001
      }
    ],
    "scenes": [
      {
        "id": "6ce3c6e4-319c-4a7a-9e88-a26e608e003d",
        "name": "Tắt phòng",
        "groups": [
          {
            "id": "2c184914-7f4f-467d-af88-59f66ea2e5b4",
            "data": {
              "onoff": 0,
              "cct": 100
            }
          }
        ]
      },
      {
        "id": "5cde677a-e140-4706-9947-7ac0ac1437e4",
        "name": "Mùa đông",
        "groups": [
          {
            "id": "2c184914-7f4f-467d-af88-59f66ea2e5b4",
            "data": {
              "onoff": 1,
              "dim": 80,
              "cct": 0
            }
          }
        ]
      },
      {
        "id": "1e970db6-c09f-42a4-be0a-3b73784e35f8",
        "name": "Mùa hè",
        "groups": [
          {
            "id": "2c184914-7f4f-467d-af88-59f66ea2e5b4",
            "data": {
              "onoff": 1,
              "dim": 100,
              "cct": 100
            }
          }
        ]
      },
      {
        "id": "3600a971-3987-4e36-897e-eff7766273b8",
        "name": "Trung tính",
        "groups": [
          {
            "id": "2c184914-7f4f-467d-af88-59f66ea2e5b4",
            "data": {
              "onoff": 1,
              "dim": 80,
              "cct": 29
            }
          }
        ]
      },
      {
        "id": "125750a2-a9d7-475e-9ef5-a45da341d6f2",
        "name": "Sinh nhật",
        "groups": [
          {
            "id": "2c184914-7f4f-467d-af88-59f66ea2e5b4",
            "data": {
              "onoff": 1,
              "dim": 100,
              "cct": 29
            }
          }
        ]
      },
      {
        "id": "9118c4de-d881-41c3-97ef-dcbb963c802e",
        "name": "Xem phim",
        "groups": [
          {
            "id": "2c184914-7f4f-467d-af88-59f66ea2e5b4",
            "data": {
              "onoff": 1,
              "dim": 30,
              "cct": 29
            }
          }
        ]
      }
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
  "cmd": "delDevToRoom",
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
    "cmd": "delDevToRoomRsp",
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
    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
    "groups": [
      "7ce30444-d762-487f-8088-7f0c16ca2207",
      "2b43f61c-9d83-494f-9ba4-7e91b2f76616"
    ],
    "scenes": [
      "2b1ca0d2-7c5b-4296-b129-767084167405",
      "cc9cbc2c-5f35-4e16-a153-44e6b6b1ba47",
      "9a2ed14a-e15f-4a9a-8f0f-bc91a8167265",
      "80ece670-5490-4319-9e0e-43a432c3968f",
      "1a654a52-fe7e-49e8-a535-6cb962ea9856",
      "9014e104-50b9-4947-bc14-9055d1a0528a"
    ],
    "rules": []
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

### II.6.5. Check data config room: server->HC
Request:
```json
{
  "cmd": "checkRoom",
  "rqi": "abc123456",
  "data": {
    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2"
  }
}
```

Response:
Bản tin phản hồi của các lệnh config room

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
### II.1.2. Request version HC: server->HC
Request:
```json
{
  "cmd": "versionHc",
  "rqi": "abc123456",
  "data": {}
}
```

Response:
```json
{
    "cmd": "versionHcRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0,
        "version":"1.0.9"
    }
}
```
### II.1.3. Hc connected cloud: app -> HC
Request:
```json
{
  "cmd": "hcConnectToCloud",
  "rpi": "abc123456",
  "data": {
    "dormitoryId": "af7d22f1-1b90-451e-8332-7c99ee08eaf2",
    "latitude": 20.9909759,
    "longitude": 105.808105
  }
}
```

Response:
```json
{
    "cmd": "hcConnectToCloudRsp",
    "rqi": "abc123456",
    "data": {
        "code": 0
    }
}
```
### II.8. Công tắc liên thông
#### II.8.1 Thêm công tắc liên thông
Request:
```json
{
  "cmd": "createSwitchLink",
  "rqi": "abc123456",
  "data": {
    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
    "lstBt": [
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      },
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      }
    ]
  }
}
```
Response:
```json
{
  "cmd": "createSwitchLinkRsp",
  "rqi": "abc123456",
  "data": {
    "code": 0,
    "success": [
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      }
    ],
    "failed": [
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      }
    ]
  }
}
```
#### II.8.2 Thêm nút nhấn vào công tắc liên thông
Request:
```json
{
  "cmd": "addBtToSwitchLink",
  "rqi": "abc123456",
  "data": {
    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
    "lstBt": [
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      },
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      }
    ]
  }
}
```
Response:
```json
{
  "cmd": "addBtToSwitchLinkRsp",
  "rqi": "abc123456",
  "data": {
    "code": 0,
    "success": [
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      }
    ],
    "failed": [
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      }
    ]
  }
}
```
#### II.8.3 Xóa nút nhấn trong công tắc liên thông
Request:
```json
{
  "cmd": "delBtFromSwitchLink",
  "rqi": "abc123456",
  "data": {
    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
    "lstBt": [
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      },
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      }
    ]
  }
}
```
Response:
```json
{
  "cmd": "delBtFromSwitchLinkRsp",
  "rqi": "abc123456",
  "data": {
    "code": 0,
    "success": [
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      }
    ],
    "failed": [
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      }
    ]
  }
}
```
#### II.8.4 Xóa công tắc liên thông
Request:
```json
{
  "cmd": "delSwitchLink",
  "rqi": "abc123456",
  "data": {
    "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2"
  }
}
```
Response:
```json
{
  "cmd": "delSwitchLinkRsp",
  "rqi": "abc123456",
  "data": {
    "code": 0,
    "success": [
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      }
    ],
    "failed": [
      {
        "id": "aa3549d4-5471-4d75-b0b2-b70fa5c10fb2",
        "bt": [
          "bt",
          "bt2"
        ]
      }
    ]
  }
}
```
## III. Bản tin giao tiếp với Android BLE

### III.1. Khởi tạo thông tin mạng BLE: AndroidBLE->HC

Request:
```json
{
  "cmd":"bleInfo",
  "rqi":"abc-xyz",
  "data":{}
}
```

Response:
```json
{
  "cmd":"bleInfoRsp",
  "rqi":"abc-xyz",
  "data":
  {
    "code":0,
    "netKey":"b717f8d8-6f18-43c0-ae46-69c32998f653",
    "appKey":"b717f8d8-6f18-43c0-ae46-69c32998f653",
    "ivIndex":"11223344",
    "addrGw": 1,
  }
}
```
### III.2. Yêu cầu quét thiết bị qua Android BLE: HC->AndroidBLE
#### Start Scan
Request:
```json
{
  "cmd": "startScanBle",
  "rqi": "abc123456",
  "data": {
    "netKey": "b717f8d8-6f18-43c0-ae46-69c32998f653",
    "appKey": "b717f8d8-6f18-43c0-ae46-69c32998f653",
    "ivIndex": "11223344",
    "addrGw": 1,
    "addrProvision": 2
  }
}
```
Response:
```json
{
    "cmd": "startScanBle",
    "rqi": "abc123456",
    "data": {
      "code":0
    }
}
```

#### Android Ble báo fast provision done -> quét thường
Request:
```json
{
  "cmd": "provisionNormal",
  "rqi":"abc123456",
  "data" : {
  }
}
```

Response:
```json
{
  "cmd": "provisionNormal",
  "rqi":"abc123456",
  "data" : {
    "code": 0
  }
}


#### Stop Scan
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
    "cmd": "stopScanBle",
    "rqi": "abc123456",
    "data": {
      "code":0
    }
}
```
### New device: AndroidBLE->HC
Request:
```json
{
  "cmd": "newDev",
  "rqi": "abc123456",
  "data": {
    "device": [
      {
        "id": "b717f8d8-6f18-43c0-ae46-69c32998f653",
        "addr": 2,
        "mac": "AB:DE:EF",
        "data": {
          "devicekey": "b717f8d8-6f18-43c0-ae46-69c32998f653",
          "vid": 768,
          "pid": 4631
        }
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
