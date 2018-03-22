#include "Hires.hpp"
#include "Debug.hpp"
#include "ImageEncoder.hpp"

#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define MAIN_PCOLOR KGRN

Hires hires(true); // save images and video
ImageEncoder imageEnc;

int main(int argc, char *argv[]) {
  assert(0 == hires.startRecording(Hires::HIRES_VID_4K, 10));
  hires.recordingAutoStop(); // 1-second loop like RTI, checking for enough frames acquired

  assert(0 == hires.startRecording(Hires::HIRES_VID_MAX_HDR, 1));
  hires.recordingAutoStop(); // 1-second loop like RTI, checking for enough frames acquired
  
  assert(0 == hires.startRecording(Hires::HIRES_VID_720P_HDR, 30));
  hires.recordingAutoStop(); // 1-second loop like RTI, checking for enough frames acquired

  assert(0 == hires.startRecording(Hires::HIRES_VID_480P, 30));
  hires.recordingAutoStop(); // 1-second loop like RTI, checking for enough frames acquired
  
  sleep(1);
  struct timeval tv;
  int stat;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(MAIN_PCOLOR "\nmain_hires; taking hires pictures at %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  hires.setPictureOutBuffer(imageEnc.getPictureInBuffer(ImageEncoder::IMGENC_IMG_2MP));
  assert(0 == hires.takePicture(Hires::HIRES_IMG_2MP));
  assert(0 == imageEnc.startJob(ImageEncoder::IMGENC_IMG_2MP));
  assert(0 == imageEnc.waitJob(2));
  assert(0 == imageEnc.writeBuffer(ImageEncoder::IMGENC_IMG_2MP,
    "MAIN_HIRES_IMGENC_HIRES_IMG_2MP"));

  assert(0 == hires.takePicture(Hires::HIRES_IMG_2MP_HDR));
  assert(0 == imageEnc.startJob(ImageEncoder::IMGENC_IMG_2MP));
  assert(0 == imageEnc.waitJob(2));
  assert(0 == imageEnc.writeBuffer(ImageEncoder::IMGENC_IMG_2MP,
    "MAIN_HIRES_IMGENC_HIRES_IMG_2MP_HDR"));

  hires.setPictureOutBuffer(NULL);
  
  assert(0 == hires.takePicture(Hires::HIRES_IMG_VGA));
  assert(0 == hires.takePicture(Hires::HIRES_IMG_VGA_HDR));
  
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(MAIN_PCOLOR "\nmain_hires; taking hires 13MP pictures at %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
  
  hires.setPictureOutBuffer(imageEnc.getPictureInBuffer(ImageEncoder::IMGENC_IMG_13MP));
  assert(0 == hires.takePicture(Hires::HIRES_IMG_13MP));
  assert(0 == imageEnc.startJob(ImageEncoder::IMGENC_IMG_13MP));
  assert(0 == imageEnc.waitJob(2));
  assert(0 == imageEnc.writeBuffer(ImageEncoder::IMGENC_IMG_13MP,
    "MAIN_HIRES_IMGENC_HIRES_IMG_13MP"));

  assert(0 == hires.takePicture(Hires::HIRES_IMG_13MP_HDR));
  assert(0 == imageEnc.startJob(ImageEncoder::IMGENC_IMG_13MP));
  assert(0 == imageEnc.waitJob(2));
  assert(0 == imageEnc.writeBuffer(ImageEncoder::IMGENC_IMG_13MP,
    "MAIN_HIRES_IMGENC_HIRES_IMG_13MP_HDR"));

  hires.setPictureOutBuffer(NULL);
  
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(MAIN_PCOLOR "\nmain_hires; taking hires raw picture at %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
  
  assert(0 == hires.takePicture(Hires::HIRES_IMG_13MP_RAW));
  
  return 0;
}
