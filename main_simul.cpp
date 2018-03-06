#include "Hires.hpp"
#include "Optic.hpp"
#include "Dual.hpp"
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

  Dual dual(true);
  
  assert(0 == dual.activateOptic());
  usleep(1000 * 500);
  
  //assert(0 == dual.activateHires());
  //sleep(2);
  assert(0 == dual.takeHiresPicture());
  sleep(2);
  dual.deactivateHires();

  return 0;
}
