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

LIBS = -lpthread -ldl -lOmxVenc -lOmxCore -lglib-2.0 -lcutils -llog #-lqcamera2 -lhardware -lqcam -lmmcamera_interface # not needed -lm -lrt -lutil

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
	LIBS := -L $(HEXAGON_ARM_SYSROOT)/usr/lib/ $(LIBS) $(HEXAGON_ARM_SYSROOT)/lib/libstdc++.so.6 $(HEXAGON_ARM_SYSROOT)/usr/lib/libcamparams.so.0 

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
LIBS := $(LIBS) # missing symlink /usr/lib/libcamparams.so.0 
test: all
	$(foreach file,$(BIN),./$(file))

test_mai%: mai%
	$<

endif # cross-compiling? or native?

AR_FLAGS=rcs

EXTRA=

mai%: mai%.cpp $(SLIB) libcamera.a
	$(CXX) $(CXXFLAGS) $(EXTRA) $< -o $@ -Wl,--start-group $(filter-out $<,$^) $(LIBS) -Wl,--end-group

LIBCAMERA_NAME = camera_memory camera_parameters qcamera2
LIBCAMERA_HDR = camera.h camera_parameters.h camera_log.h camera_memory.h qcamera2.h qcamera_extensions.h QCamera2_Ext.h
LIBCAMERA_CXXFLAGS = -w -I $(HEXAGON_ARM_SYSROOT)/usr/src/kernel/include/
LIBCAMERA_SRC = $(foreach name,$(sort $(LIBCAMERA_NAME)),$(name).cpp)
LIBCAMERA_OBJ = $(foreach name,$(sort $(LIBCAMERA_NAME)),$(name).o)
libcamera.a: $(LIBCAMERA_SRC) $(LIBCAMERA_HDR)
	$(CXX) $(CXXFLAGS) $(LIBCAMERA_CXXFLAGS) -c -o camera_memory.o camera_memory.cpp 
	$(CXX) $(CXXFLAGS) $(LIBCAMERA_CXXFLAGS) -c -o camera_parameters.o camera_parameters.cpp
	$(CXX) $(CXXFLAGS) $(LIBCAMERA_CXXFLAGS) -c -o qcamera2.o qcamera2.cpp
	$(AR) $(AR_FLAGS) $@ $(LIBCAMERA_OBJ)

LIBQCAMERA_HDR = 
LIBQCAMERA_NAME = QCamera2Factory QCamera2Hal QCamera2HWI QCameraMem \
	QCameraQueue QCameraCmdThread QCameraStateMachine QCameraChannel \
	QCameraStream QCameraPostProc QCamera2HWICallbacks QCameraParameters QCameraThermalAdapter
LIBQCAMERA_SRC = $(foreach name,$(sort $(LIBQCAMERA_NAME)),qcamera2/$(name).cpp)
LIBQCAMERA_OBJ = $(foreach name,$(sort $(LIBQCAMERA_NAME)),qcamera2/$(name).o)
LIBQCAMERA_CXXFLAGS = -w -I $(HEXAGON_ARM_SYSROOT)/usr/src/kernel/include/ -I qcamera2 \
	-I $(HEXAGON_ARM_SYSROOT)/usr/include/mm_camera_interface/ \
	-I $(HEXAGON_ARM_SYSROOT)/usr/include/glib-2.0/ \
	-I $(HEXAGON_ARM_SYSROOT)/usr/lib/glib-2.0/include/ \
	-I $(HEXAGON_ARM_SYSROOT)/usr/include/camera-hal/ \
	-I $(HEXAGON_ARM_SYSROOT)/usr/include/ \
	-I $(HEXAGON_ARM_SYSROOT)/usr/include/omx

qcamera2/%.o: qcamera2/%.cpp $(LIBQCAMERA_HDR)
	$(CXX) $(LIBQCAMERA_CXXFLAGS) -c -o $@ $<

libqcamera2.a: $(LIBQCAMERA_OBJ)
	$(AR) $(AR_FLAGS) $@ $(LIBQCAMERA_OBJ)

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
	rm $(OBJ) $(SLIB) $(BIN) libcamera.a
