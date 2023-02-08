VERSION = 2.0.0

ZIGBEE = OFF

CC ?= gcc
CXX ?= g++
OBJEXT ?= .o
BUILD_PATH = build

INCLUDES = -I. -Ibutton -Iconfig -Idatabase -Idevice -Idevice/ble -Igateway -Igroup -Ijson -Ilog -Imqtt -Iprotocol/ble -Irule -IsceneBle -Iuart -Iutil -Iwifi
COMPFLAGS =  -Wall -std=c++17 -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -Wno-deprecated -Wno-deprecated-declarations -Wno-unused-result -flto -fPIC
COMPFLAGS += -DVERSION=$(VERSION)
LINKFLAGS =  -Wall -std=c++17 -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -flto

LINKEDLIBS = -lpthread -lmosquitto -lsqlite3 -luci


ifeq ($(ZIGBEE),ON)
	INCLUDES 	+= -Idevice/zigbee -Iprotocol/zigbee
	COMPFLAGS += -DCONFIG_ENABLE_ZIGBEE=1

	DEVICESRC += $(wildcard protocol/zigbee/*.cpp)
	DEVICESRC += $(wildcard device/zigbee/*.cpp)
	DEVICESRC += $(wildcard device/zigbee/cluster/*.cpp)
	DEVICESRC += $(wildcard device/zigbee/cluster/onoff/*.cpp)
	DEVICESRC += $(wildcard device/zigbee/cluster/onoff/attribute/*.cpp)
endif

DEVICESRC += $(wildcard button/*.cpp)
DEVICESRC += $(wildcard config/*.cpp)
DEVICESRC += $(wildcard database/*.cpp)
DEVICESRC += $(wildcard device/*.cpp)
DEVICESRC += $(wildcard device/ble/*.cpp)
DEVICESRC += $(wildcard device/ble/module/*.cpp)
DEVICESRC += $(wildcard device/ble/element/*.cpp)
DEVICESRC += $(wildcard gateway/*.cpp)
DEVICESRC += $(wildcard group/*.cpp)
DEVICESRC += $(wildcard json/*.cpp)
DEVICESRC += $(wildcard log/*.cpp)
DEVICESRC += $(wildcard mqtt/*.cpp)
DEVICESRC += $(wildcard protocol/ble/*.cpp)
DEVICESRC += $(wildcard rule/*.cpp)
DEVICESRC += $(wildcard sceneBle/*.cpp)
DEVICESRC += $(wildcard uart/*.cpp)
DEVICESRC += $(wildcard util/*.cpp)
DEVICESRC += $(wildcard wifi/*.cpp)

CPPSRC = $(wildcard *.cpp) $(DEVICESRC)
CPPOBJ = $(CPPSRC:.cpp=$(OBJEXT))
BUILTOBJ = $(addprefix $(BUILD_PATH)/,$(CPPOBJ))

APP = smh

all: $(APP)

.PHONY: all $(APP) clean

$(APP): $(BUILTOBJ)
	$(CXX) $(LINKFLAGS) -o $@ $(BUILTOBJ) $(LINKEDLIBS)
	
$(BUILD_PATH)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(COMPFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_PATH)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(COMPFLAGS) $(INCLUDES) -c $< -o $@
	
install: $(APP)
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 644 $(APP) $(DESTDIR)$(PREFIX)/bin/

clean: 
	rm -rf $(APP) $(BUILD_PATH)
