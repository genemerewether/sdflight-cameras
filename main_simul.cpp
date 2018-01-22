#include "Hires.hpp"
#include "Optic.hpp"
#include "Debug.hpp"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define MAIN_PCOLOR KGRN

int main(int argc, char *argv[]) {
  struct timeval tv;
  int stat;

  Hires hires;
  Optic optic;

  assert(0 == optic.activate());
  usleep(1000 * 500);

  gettimeofday(&tv,NULL);
  DEBUG_PRINT(MAIN_PCOLOR "\nmain_simul; taking hires picture at %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
  stat = hires.takePicture();
  if (stat) {
    gettimeofday(&tv,NULL);
    DEBUG_PRINT(KRED "\nmain_simul; hires takePicture failed with value %d at %f\n" KNRM,
                stat, tv.tv_sec + tv.tv_usec / 1000000.0);
  }
  assert(stat == 0);
  sleep(1);
  assert(0 == hires.startRecording());
  usleep(1000 * 500);
  hires.stopRecording();
  usleep(1000 * 500);

  optic.deactivate();

  return 0;
}
