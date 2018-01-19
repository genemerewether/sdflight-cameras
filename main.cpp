#include "Hires.hpp"
#include "Optic.hpp"
#include "Debug.hpp"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>

Hires hires;

int main(int argc, char *argv[]) {
  assert(0 == hires.takePicture());
  return 0;
}
