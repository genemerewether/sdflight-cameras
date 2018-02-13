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
  struct timeval tv;
  int stat;

//   gettimeofday(&tv,NULL);
//   DEBUG_PRINT(MAIN_PCOLOR "\nmain_hires; taking hires pictures at %f\n" KNRM,
//               tv.tv_sec + tv.tv_usec / 1000000.0);
//   stat = hires.takePicture(Hires::HIRES_IMG_2MP);
//   assert(stat == 0);
//   stat = hires.takePicture(Hires::HIRES_IMG_2MP_HDR);
//   assert(stat == 0);
//   stat = hires.takePicture(Hires::HIRES_IMG_13MP_RAW);
//   assert(stat == 0);
//   stat = hires.takePicture(Hires::HIRES_IMG_13MP);
//   assert(stat == 0);
//   stat = hires.takePicture(Hires::HIRES_IMG_13MP_HDR);
//   assert(stat == 0);
//   stat = hires.takePicture(Hires::HIRES_IMG_QVGA);
//   assert(stat == 0);
//   stat = hires.takePicture(Hires::HIRES_IMG_QVGA_HDR);
//   assert(stat == 0);

  /*  if (stat) {
    gettimeofday(&tv,NULL);
    DEBUG_PRINT(KRED "\nmain_hires; hires takePicture failed with value %d at %f\n" KNRM,
                stat, tv.tv_sec + tv.tv_usec / 1000000.0);
                }*/
  sleep(1);

  assert(0 == hires.startRecording(Hires::HIRES_VID_4K, 10));
  hires.recordingAutoStop(); // 1-second loop like RTI, checking for enough frames acquired

  assert(0 == hires.startRecording(Hires::HIRES_VID_4K, 1));
  hires.recordingAutoStop(); // 1-second loop like RTI, checking for enough frames acquired

  assert(0 == hires.startRecording(Hires::HIRES_VID_MAX_HDR));
  usleep(1000 * 100);
  hires.stopRecording();
  assert(0 == hires.startRecording(Hires::HIRES_VID_1080P));
  usleep(1000 * 100);
  hires.stopRecording();
  assert(0 == hires.startRecording(Hires::HIRES_VID_1080P_HDR));
  usleep(1000 * 100);
  hires.stopRecording();

  return 0;
}
