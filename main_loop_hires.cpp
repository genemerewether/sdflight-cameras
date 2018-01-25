#include "Hires.hpp"
#include "Debug.hpp"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

Hires hires;

int main(int argc, char *argv[]) {
  printf("Press any key to start infinite loop\n");
  (void) getchar();
  while (1) {
    assert(0 == hires.takePicture());
    sleep(1);
    assert(0 == hires.startRecording());
    usleep(1000 * 500);
    hires.stopRecording();
    sleep(1);
  }

  return 0;
}