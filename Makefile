SRC = Hires Optic

BIN = main # main_thread

DEPS = $(foreach name,$(SRC),$(name).hpp) Debug.hpp
OBJ = $(foreach name,$(sort $(SRC) $(BIN)),$(name).o)
SLIB = $(foreach name,$(SRC),lib$(name).a)

PUSHDIR = /home/linaro/camtest/

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

# cross-compiling? or native?
uname_m = $(shell uname -m)
ifneq (,$(filter $(uname_m),x86_64 x86)) # cross-compiling

	CXX = $(HEXAGON_SDK_ROOT)/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf_linux/bin/arm-linux-gnueabihf-g++
	AR = $(HEXAGON_SDK_ROOT)/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf_linux/bin/arm-linux-gnueabihf-ar
	CXXFLAGS := $(CXXFLAGS) -I$(HEXAGON_SDK_ROOT)/incs
	LIBS := -L $(HEXAGON_ARM_SYSROOT)/usr/lib/ $(LIBS) $(HEXAGON_ARM_SYSROOT)/lib/libstdc++.so.6

load: all
	$(foreach file,$(BIN),adb push $(file) $(PUSHDIR)$(file))

test: load
	$(foreach file,$(BIN),adb shell $(PUSHDIR)$(file))

else # native

CXX = /usr/bin/g++
AR = /usr/bin/ar
test: all
	$(foreach file,$(BIN),./$(file))

endif # cross-compiling? or native?

AR_FLAGS=rcs

EXTRA=

mai%: mai%.cpp $(SLIB)
	$(CXX) $(CXXFLAGS) $(EXTRA) $< -o $@ -Wl,--start-group $(filter-out $<,$^) $(LIBS) -Wl,--end-group

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

lib%.a: %.o $(DEPS)
	$(AR) $(AR_FLAGS) $@ $<

.PHONY: all clean print load test

print:
	@echo "OBJ: " $(OBJ)
	@echo "SLIB: " $(SLIB)
	@echo "DEPS: " $(DEPS)
	@echo "BIN: " $(BIN)
	@echo "LIBS: " $(LIBS)

clean:
	rm $(OBJ) $(SLIB) $(BIN)
