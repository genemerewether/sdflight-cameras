#include "Optic.hpp"
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#define OPTIC_PCOLOR KYEL

#define OPTIC_CAM_TYPE 1

Optic::Optic(bool postProc, bool print) :
  m_print(print),
  m_cameraPtr(NULL),
  m_params()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(OPTIC_PCOLOR "\nOptic constructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  int stat = 0;
  int cameraID = 0;
  bool found = false;
  int numCameras = camera::getNumberOfCameras();
  assert(numCameras > 0);

  for (int i = 0; i < numCameras; i++) {
    camera::CameraInfo cameraInfo;
    stat = camera::getCameraInfo(i, cameraInfo);
    assert(stat == 0);

    if (cameraInfo.func == static_cast<int>(OPTIC_CAM_TYPE))
    {
      cameraID = i;
      found = true;
    }
  }

  assert(found);

  stat = camera::ICameraDevice::createInstance(cameraID, &m_cameraPtr);
  assert(stat == 0);

  stat = m_params.init(m_cameraPtr);
  assert(stat == 0);

  if (postProc) {
    printf(OPTIC_PCOLOR "\nUsing 8-bit callback in Optic\n" KNRM);
  }
  else {
    printf(OPTIC_PCOLOR "\nUsing 10-bit callback in Optic\n" KNRM);
    m_params.set("preview-format", "bayer-rggb");
    // even though we do not use takePicture callback, NEED to set this
    m_params.set("picture-format", "bayer-mipi-10gbrg");
    m_params.set("raw-size", "640x480");
    stat = m_params.commit();
    assert(stat == 0);
  }

  assert(0 == pthread_mutex_init(&m_cameraFrameLock, 0));
}

Optic::~Optic() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(OPTIC_PCOLOR "\nOptic destructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  if(m_cameraPtr != NULL) {
    camera::ICameraDevice::deleteInstance(&m_cameraPtr);
    m_cameraPtr = NULL;
  }

  assert(0 == pthread_mutex_destroy(&m_cameraFrameLock));
}

// Interface functions
void Optic::onError() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(OPTIC_PCOLOR "\nOptic Error callback at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
}

void Optic::onControl(const camera::ControlEvent& control) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(OPTIC_PCOLOR "\nOptic Control callback type %d at time %f\n" KNRM,
              control.type,
              tv.tv_sec + tv.tv_usec / 1000000.0);
}

void Optic::onPreviewFrame(camera::ICameraFrame *frame) {
  if (m_print) {
    static struct timeval tv_last;
    static bool first = true;
    struct timeval tv;
    gettimeofday(&tv,NULL);
    /*DEBUG_PRINT(OPTIC_PCOLOR "\nOptic Preview callback at time %f; size %u\n" KNRM,
      tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);*/
    if (first) {
      first = false;
    }
    else {
      double elapsed = tv.tv_sec - tv_last.tv_sec
	+ tv.tv_usec / 1000000.0 - tv_last.tv_usec / 1000000.0;
      /*DEBUG_PRINT(OPTIC_PCOLOR "\nOptic preview callback elapsed time %f\n" KNRM,
	elapsed);*/
      if ((elapsed < 0.020) || (elapsed > 0.040)) {
        DEBUG_PRINT(OPTIC_PCOLOR "\nOptic preview callback elapsed time %f\n" KNRM,
		    elapsed);
      }
    }
    tv_last = tv;
  }
}

void Optic::onVideoFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(OPTIC_PCOLOR "\nOptic Video callback at time %f; size %u\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
}

void Optic::onPictureFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(OPTIC_PCOLOR "\nOptic Picture callback at time %f; size %u\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
}

void Optic::onMetadataFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(OPTIC_PCOLOR "\nOptic Metadata callback at time %f; size %u\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
}

// Private functions
int Optic::activate() {
  assert(m_cameraPtr != NULL);
  m_cameraPtr->addListener(this);
  return m_cameraPtr->startPreview();
}

void Optic::deactivate() {
  assert(m_cameraPtr != NULL);
  m_cameraPtr->removeListener(this);
  m_cameraPtr->stopPreview();
  return;
}
