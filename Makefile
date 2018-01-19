SRC = Hires #Optic

BIN = main # main_nothread main_thread main_hires main_optic

DEPS = $(foreach name,$(SRC),$(name).hpp) Debug.hpp
OBJ = $(foreach name,$(sort $(SRC) $(BIN)),$(name).o)
SLIB = $(foreach name,$(SRC),lib$(name).a)

CXX = /usr/bin/g++
AR = /usr/bin/ar
CXXFLAGS = -I. -g

LIBS = -lpthread -lcamera -lcamparams

uname_p = $(shell uname -p)
ifneq (,$(filter $(uname_p),x86_64 x86))
	CXX = $(HEXAGON_SDK_ROOT)/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf_linux/bin/arm-linux-gnueabihf-g++
	AR = $(HEXAGON_SDK_ROOT)/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf_linux/bin/arm-linux-gnueabihf-ar
	CXXFLAGS := $(CXXFLAGS) -I$(HEXAGON_SDK_ROOT)/incs
	LIBS := -L $(HEXAGON_ARM_SYSROOT)/usr/lib/ $(LIBS) $(HEXAGON_ARM_SYSROOT)/lib/libstdc++.so.6
endif

AR_FLAGS=rcs

EXTRA=

all: $(BIN)

main: $(SLIB) main.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA) -o $@ -Wl,--start-group $^ $(LIBS) -Wl,--end-group

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

lib%.a: %.o $(DEPS)
	$(AR) $(AR_FLAGS) $@ $<

.PHONY: all clean print

print:
	@echo "OBJ: " $(OBJ)
	@echo "SLIB: " $(SLIB)
	@echo "DEPS: " $(DEPS)
	@echo "BIN: " $(BIN)
	@echo "LIBS: " $(LIBS)

clean:
	rm $(OBJ) $(SLIB) $(BIN)
