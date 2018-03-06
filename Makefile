SRC = Hires Optic Encoder Dual

BIN = main_hires main_optic main_optic_nostop main_simul main_simul_gbl # main main_loop # main_thread

DEPS = Debug.hpp EncoderConfig.hpp
HDR = $(foreach name,$(SRC),$(name).hpp) $(DEPS)
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
	-Wnon-virtual-dtor \
	-std=c++03 # \
	#-std=gnu++0x

LIBS = -lpthread -lcamera -lOmxVenc -lOmxCore -lglib-2.0 -lcutils -llog # missing symlink -lcamparams # not needed -ldl -lm -lrt -lutil

all: $(BIN)

# cross-compiling? or native?
uname_m = $(shell uname -m)
ifneq (,$(filter $(uname_m),x86_64 x86)) # cross-compiling

	CXX = $(HEXAGON_SDK_ROOT)/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf_linux/bin/arm-linux-gnueabihf-g++
	AR = $(HEXAGON_SDK_ROOT)/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf_linux/bin/arm-linux-gnueabihf-ar
	CXXFLAGS := $(CXXFLAGS) -I$(HEXAGON_SDK_ROOT)/incs \
				-I$(HEXAGON_SDK_ROOT)/incs/stddef \
				-I$(HEXAGON_SDK_ROOT)/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf_linux/arm-linux-gnueabihf/include/c++/4.9.3/ \
				-I$(HEXAGON_ARM_SYSROOT)/usr/include/omx \
				-I$(HEXAGON_ARM_SYSROOT)/usr/include/ \
				-D__GLIBC_HAVE_LONG_LONG
	LIBS := -L $(HEXAGON_ARM_SYSROOT)/usr/lib/ $(HEXAGON_ARM_SYSROOT)/usr/lib/libcamparams.so.0 $(LIBS) $(HEXAGON_ARM_SYSROOT)/lib/libstdc++.so.6

test_mai%: mai%
	adb push $< $(PUSHDIR)$<
	adb shell 'mkdir -p /home/linaro/tmp && cd /home/linaro/tmp && $(PUSHDIR)$<'

test_multiproc: main_optic main_hires
	$(foreach file,$^,adb push $(file) $(PUSHDIR)$(file);)
	adb shell '($(PUSHDIR)main_optic &); sleep 1; $(PUSHDIR)main_hires; sleep 10'

test_loop: main_loop
	adb push main_loop $(PUSHDIR)main_loop
	adb shell 'screen -S LOOPTESTMAIN taskset -c 2 $(PUSHDIR)main_loop'
	adb shell screen -dD

test_loop_multiproc: main_loop_optic main_loop_hires
	$(foreach file,$^,adb push $(file) $(PUSHDIR)$(file);)
	adb shell 'screen -S BOTHLOOPTESTMAIN -t OPTIC taskset -c 2 $(PUSHDIR)main_loop_optic'
	adb shell screen -dD
	adb shell 'screen -S BOTHLOOPTESTMAIN -X screen -t HIRES taskset -c 2 $(PUSHDIR)main_loop_hires'

load: all
	$(foreach file,$(BIN),adb push $(file) $(PUSHDIR)$(file);)

test: load
	$(foreach file,$(BIN),adb shell $(PUSHDIR)$(file);)

else # native

CXX = /usr/bin/g++
CXXFLAGS = $(CXXFLAGS) -I/usr/include/omx
AR = /usr/bin/ar
LIBS := /usr/lib/libcamparams.so.0 $(LIBS) # missing symlink
test: all
	$(foreach file,$(BIN),./$(file))

test_mai%: mai%
	$<

endif # cross-compiling? or native?

AR_FLAGS=rcs

EXTRA=

mai%: mai%.cpp $(SLIB)
	$(CXX) $(CXXFLAGS) $(EXTRA) $< -o $@ -Wl,--start-group $(filter-out $<,$^) $(LIBS) -Wl,--end-group

%.o: %.cpp %.hpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

libEncoder.a: Encoder.o EncoderConfig.o $(DEPS)
	$(AR) $(AR_FLAGS) $@ $< EncoderConfig.o

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
