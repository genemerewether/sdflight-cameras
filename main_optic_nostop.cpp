#include "Optic.hpp"
#include "Debug.hpp"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define MAIN_PCOLOR KGRN

Optic optic(true);

int main(int argc, char *argv[]) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(MAIN_PCOLOR "\nmain_optic; activating optic camera at %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
  assert(0 == optic.activate());
  while (1) {
    sleep(1);
  }

  return 0;
}
