#include "Hires.hpp"
#include "Debug.hpp"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define MAIN_PCOLOR KGRN

Hires hires;

int main(int argc, char *argv[]) {
  struct timeval tv;
  int stat;

  gettimeofday(&tv,NULL);
  DEBUG_PRINT(MAIN_PCOLOR "\nmain_hires; taking hires picture at %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
  stat = hires.takePicture();
  if (stat) {
    gettimeofday(&tv,NULL);
    DEBUG_PRINT(KRED "\nmain_hires; hires takePicture failed with value %d at %f\n" KNRM,
                stat, tv.tv_sec + tv.tv_usec / 1000000.0);
  }
  assert(stat == 0);
  sleep(1);
  assert(0 == hires.startRecording());
  usleep(1000 * 500);
  hires.stopRecording();

  return 0;
}
