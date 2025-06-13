CC ?= gcc
CXX ?= g++
OBJEXT ?= .o

# Đưa build ra ngoài thư mục src
BUILD_PATH = ../build
APP = ../smh

CFLAGS = -Wno-unused-function -fno-integrated-as -fstrict-aliasing -fPIC -Os -ffunction-sections -fdata-sections -MD
CXXFLAGS = -std=c++17 -Os -ffunction-sections -fdata-sections -Wno-unused-result -Wno-deprecated-declarations -MD
LDFLAGS = -Wl,--gc-sections -Os -ffunction-sections -fdata-sections

INCLUDES = -I.. -I../button -I../config -I../object -I../device -I../device/ble -I../gateway -I../json -I../log -I../mqtt -I../protocol/ble -I../protocol/qr -I../protocol/gpio -I../protocol/relay -I../uart -I../util -I../wifi

LINKEDLIBS = -lmosquittopp -lsqlite3 -pthread -lcurl -lssl -lcrypto

DEVICESRC += $(wildcard ../button/*.cpp)
DEVICESRC += $(wildcard ../config/*.cpp)
DEVICESRC += $(wildcard ../object/*.cpp)
DEVICESRC += $(wildcard ../device/*.cpp)
DEVICESRC += $(wildcard ../device/ble/*.cpp)
DEVICESRC += $(wildcard ../device/ble/module/*.cpp)
DEVICESRC += $(wildcard ../gateway/*.cpp)
DEVICESRC += $(wildcard ../json/*.cpp)
DEVICESRC += $(wildcard ../log/*.cpp)
DEVICESRC += $(wildcard ../mqtt/*.cpp)
DEVICESRC += $(wildcard ../protocol/ble/*.cpp)
DEVICESRC += $(wildcard ../protocol/relay/*.cpp)
DEVICESRC += $(wildcard ../protocol/qr/*.cpp)
DEVICESRC += $(wildcard ../protocol/gpio/*.cpp)
DEVICESRC += $(wildcard ../uart/*.cpp)
DEVICESRC += $(wildcard ../util/*.cpp)
DEVICESRC += $(wildcard ../wifi/*.cpp)

TESTSRC = $(wildcard *.cpp) $(DEVICESRC)
TESTOBJ = $(TESTSRC:.cpp=$(OBJEXT))
BUILTOBJ = $(addprefix $(BUILD_PATH)/,$(TESTOBJ))

all: $(APP)

.PHONY: all $(APP) clean

$(APP): $(BUILTOBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) -o $@ $(BUILTOBJ) $(LINKEDLIBS)

$(BUILD_PATH)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -DUNIT_TEST -c $< -o $@

install: $(APP)
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 744 $(APP) $(DESTDIR)$(PREFIX)/bin/

clean:
	rm -rf $(APP) $(BUILD_PATH)

-include $(BUILTOBJ:.o=.d)
