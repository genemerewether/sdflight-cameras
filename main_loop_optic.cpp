#include "Optic.hpp"
#include "Debug.hpp"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

Optic optic;

int main(int argc, char *argv[]) {
  printf("Press any key to start infinite loop\n");
  (void) getchar();
  while (1) {
    assert(0 == optic.activate());
    usleep(1000 * 500);
    optic.deactivate();
    sleep(1);
  }

  return 0;
}
