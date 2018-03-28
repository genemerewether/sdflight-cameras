#include "Hires.hpp"
#include "Optic.hpp"
#include "Debug.hpp"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

Hires hires;
Optic optic(false);

int main(int argc, char *argv[]) {
  printf("Press any key to start infinite loop\n");
  (void) getchar();
  
  assert(0 == optic.activate());
  
  assert(0 == hires.takePicture(Hires::HIRES_IMG_13MP_RAW));
  sleep(1);
  
  while (1) {
    assert(0 == hires.startRecording());
    usleep(1000 * 500);
    hires.stopRecording();
    sleep(1);
    assert(0 == hires.takePicture(Hires::HIRES_IMG_13MP_RAW));
    sleep(1);
  }

  return 0;
}
