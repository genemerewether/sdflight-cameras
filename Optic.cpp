#include "Optic.hpp"
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#define OPTIC_PCOLOR KYEL

#define OPTIC_CAM_TYPE 1

Optic::Optic(bool print) :
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
}

void Optic::onPreviewFrame(camera::ICameraFrame *frame) {
  if (m_print) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    DEBUG_PRINT(OPTIC_PCOLOR "\nOptic Preview callback at time %f; size %u\n" KNRM,
                tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
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
