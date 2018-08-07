#include "Hires.hpp"
#include "Optic.hpp"
#include "Debug.hpp"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

Hires hires(true); // always cycle

volatile sig_atomic_t terminate = 0;

static void sighandler(int signum) {
    terminate = 1;
}

int main(int argc, char *argv[]) {
  signal(SIGINT,sighandler);
  while (1) {
    printf("Press any key to start infinite loop; SIGTERM to terminate\n");
    (void) getchar();
    terminate = 0;

    //assert(0 == hires.takePicture(Hires::HIRES_IMG_13MP_RAW));
    //sleep(1);

    while (!terminate) {
      /*assert(0 == hires.startRecording());
      usleep(1000 * 500);
      hires.stopRecording();
      sleep(1);*/

      assert(0 == hires.takePicture(Hires::HIRES_IMG_2MP));

      assert(0 == hires.takePicture(Hires::HIRES_IMG_JPG_TEST));
      //sleep(1);
    }
  }

  return 0;
}
