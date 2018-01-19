SRC = Hires #Optic

BIN = main # main_nothread main_thread main_hires main_optic

DEPS = $(foreach name,$(SRC),$(name).hpp) Debug.hpp
OBJ = $(foreach name,$(sort $(SRC) $(BIN)),$(name).o)
SLIB = $(foreach name,$(SRC),lib$(name).a)

PUSHDIR = /home/linaro/camtest/

CXX = /usr/bin/g++
AR = /usr/bin/ar
CXXFLAGS = -I. -g \
	-Wall -Wextra \
	-fno-builtin -fno-asm \
        -fstrength-reduce \
	-Wno-unused-parameter \
	-Wno-long-long \
	-fcheck-new \
	-Wnon-virtual-dtor

LIBS = -lpthread -lcamera -lcamparams #-ldl -lm -lrt -lutil

all: $(BIN)

uname_p = $(shell uname -p)
ifneq (,$(filter $(uname_p),x86_64 x86))
	CXX = $(HEXAGON_SDK_ROOT)/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf_linux/bin/arm-linux-gnueabihf-g++
	AR = $(HEXAGON_SDK_ROOT)/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf_linux/bin/arm-linux-gnueabihf-ar
	CXXFLAGS := $(CXXFLAGS) -I$(HEXAGON_SDK_ROOT)/incs
	LIBS := -L $(HEXAGON_ARM_SYSROOT)/usr/lib/ $(LIBS) $(HEXAGON_ARM_SYSROOT)/lib/libstdc++.so.6

load: all
	$(foreach file,$(BIN),adb push $(file) $(PUSHDIR)$(file))

test: load
	$(foreach file,$(BIN),adb shell $(PUSHDIR)$(file))
else
test: all
	$(foreach file,$(BIN),./$(file))
endif

AR_FLAGS=rcs

EXTRA=

main: $(SLIB) main.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA) -o $@ -Wl,--start-group $^ $(LIBS) -Wl,--end-group

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

lib%.a: %.o $(DEPS)
	$(AR) $(AR_FLAGS) $@ $<

.PHONY: all clean print load

print:
	@echo "OBJ: " $(OBJ)
	@echo "SLIB: " $(SLIB)
	@echo "DEPS: " $(DEPS)
	@echo "BIN: " $(BIN)
	@echo "LIBS: " $(LIBS)

clean:
	rm $(OBJ) $(SLIB) $(BIN)
