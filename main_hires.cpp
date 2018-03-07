#include "Hires.hpp"
#include "Debug.hpp"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define MAIN_PCOLOR KGRN

Hires hires(true);

int main(int argc, char *argv[]) {
  assert(0 == hires.startRecording(Hires::HIRES_VID_4K, 10));
  hires.recordingAutoStop(); // 1-second loop like RTI, checking for enough frames acquired

  assert(0 == hires.startRecording(Hires::HIRES_VID_MAX_HDR, 1));
  hires.recordingAutoStop(); // 1-second loop like RTI, checking for enough frames acquired
  
  assert(0 == hires.startRecording(Hires::HIRES_VID_720P_HDR, 30));
  hires.recordingAutoStop(); // 1-second loop like RTI, checking for enough frames acquired

  assert(0 == hires.startRecording(Hires::HIRES_VID_480P, 30));
  hires.recordingAutoStop(); // 1-second loop like RTI, checking for enough frames acquired
  
  sleep(1);
  struct timeval tv;
  int stat;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(MAIN_PCOLOR "\nmain_hires; taking hires pictures at %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
  
  stat = hires.takePicture(Hires::HIRES_IMG_2MP);
  assert(stat == 0);
  stat = hires.takePicture(Hires::HIRES_IMG_2MP_HDR);
  assert(stat == 0);
  
  stat = hires.takePicture(Hires::HIRES_IMG_QVGA);
  assert(stat == 0);
  stat = hires.takePicture(Hires::HIRES_IMG_QVGA_HDR);
  assert(stat == 0);
  
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(MAIN_PCOLOR "\nmain_hires; taking hires 13MP pictures at %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
  
  stat = hires.takePicture(Hires::HIRES_IMG_13MP);
  assert(stat == 0);
  stat = hires.takePicture(Hires::HIRES_IMG_13MP_HDR);
  assert(stat == 0);
  
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(MAIN_PCOLOR "\nmain_hires; taking hires raw picture at %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
  
  stat = hires.takePicture(Hires::HIRES_IMG_13MP_RAW);
  assert(stat == 0);
  
  return 0;
}
