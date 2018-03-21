#include "Hires.hpp"
#include "Optic.hpp"
#include "Debug.hpp"
#include "ImageEncoder.hpp"
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define MAIN_PCOLOR KGRN

Hires hires;
Optic optic(false); // don't print
ImageEncoder imageEnc;

int main(int argc, char *argv[]) {
  assert(0 == optic.activate());
  usleep(1000 * 500);

  hires.setPictureOutBuffer(imageEnc.getPictureInBuffer(ImageEncoder::IMGENC_IMG_13MP));
  assert(0 == hires.takePicture(Hires::HIRES_IMG_13MP));
  assert(0 == imageEnc.startJob(ImageEncoder::IMGENC_IMG_13MP));
  assert(0 == imageEnc.waitJob(2));
  /*assert(0 == imageEnc.writeBuffer(ImageEncoder::IMGENC_IMG_13MP,
    "SIMUL_GBL_IMGENC_HIRES_IMG_13MP"));*/

  hires.setPictureOutBuffer(imageEnc.getPictureInBuffer(ImageEncoder::IMGENC_IMG_2MP));
  assert(0 == hires.takePicture(Hires::HIRES_IMG_2MP));
  assert(0 == imageEnc.startJob(ImageEncoder::IMGENC_IMG_2MP));
  assert(0 == imageEnc.waitJob(2));
  /*assert(0 == imageEnc.writeBuffer(ImageEncoder::IMGENC_IMG_2MP,
    "SIMUL_GBL_IMGENC_HIRES_IMG_2MP"));*/

  sleep(1);
  assert(0 == hires.startRecording());
  usleep(1000 * 500);
  hires.stopRecording();
  usleep(1000 * 500);

  optic.deactivate();

  return 0;
}
