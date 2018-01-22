## Makefile

### Environment (cross-compiling)
- Make sure `HEXAGON_SDK_ROOT` and `HEXAGON_ARM_SYSROOT` are defined
- `make`

### Environment (native)
- no special setup needed
```
adb push ../sdflight-cameras /home/linaro/sdflight-cameras
adb shell
cd /home/linaro/sdflight-cameras
make
```

## Use-cases that are being debugged

### Multi-process
  - This successful test operates 4K and optic flow cameras in separate processes

### Single process, accessing camera API in main thread
  - This test, which tries to take a picture using the 4K camera while optic flow preview callbacks are occuring, fails.

### Running tests (remote, over adb)
- `make test_multiproc`

### Running tests (native)
```
./main_optic &
./main_hires
```

### Overall use case: optic flow and 4K camera simultaneous capture
- optic flow camera onPreviewFrame callbacks always on
- 4K camera takePicture intermittently
- 4K camera startRecording and stopRecording intermittently

### Code design / classes
- One class for 4K camera, one for optic flow camera
- Both declared at global scope (doesn't seem to matter)
