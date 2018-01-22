#include "Hires.hpp"
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#define HIRES_CAM_TYPE 0

Hires::Hires() :
  m_cameraPtr(NULL),
  m_params(),
  m_frameReady(false),
  m_recording(false)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT("\nHires constructor at time %f\n",
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

    if (cameraInfo.func == static_cast<int>(HIRES_CAM_TYPE))
    {
      cameraID = i;
      found = true;
    }
  }

  assert(found);

  stat = camera::ICameraDevice::createInstance(cameraID, &m_cameraPtr);
  assert(stat == 0);

  assert(0 == pthread_mutex_init(&m_cameraFrameLock, 0));

  assert(0 == pthread_cond_init(&m_cameraFrameReady, 0));
}

Hires::~Hires() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT("\nHires destructor at time %f\n",
              tv.tv_sec + tv.tv_usec / 1000000.0);

  if(m_cameraPtr != NULL) {
    camera::ICameraDevice::deleteInstance(&m_cameraPtr);
    m_cameraPtr = NULL;
  }

  assert(0 == pthread_mutex_destroy(&m_cameraFrameLock));

  assert(0 == pthread_cond_destroy(&m_cameraFrameReady));
}

int Hires::takePicture() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  if (m_recording) {
    DEBUG_PRINT("\nHires takePicture called while recording at time %f\n",
                tv.tv_sec + tv.tv_usec / 1000000.0);
    return -1;
  }
  DEBUG_PRINT("\nHires takePicture called at time %f\n",
              tv.tv_sec + tv.tv_usec / 1000000.0);

  int stat = this->activate();
  if (stat) {
    return stat;
  }

  stat = m_cameraPtr->takePicture();
  if (stat) {
    return stat;
  }
  assert(0 == pthread_mutex_lock(&m_cameraFrameLock));
  m_frameReady = false;

  while (!m_frameReady) {
    assert(0 == pthread_cond_wait(&m_cameraFrameReady, &m_cameraFrameLock));
  }

  assert(0 == pthread_mutex_unlock(&m_cameraFrameLock));
  this->deactivate();

  return 0;
}

int Hires::startRecording() {
  int stat = this->activate();
  if (stat) {
    return stat;
  }
  return m_cameraPtr->startRecording();
}

void Hires::stopRecording() {
  m_cameraPtr->stopRecording();
  this->deactivate();
}

// Interface functions
void Hires::onError() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT("\nHires Error callback at time %f\n",
              tv.tv_sec + tv.tv_usec / 1000000.0);
}

void Hires::onControl(const camera::ControlEvent& control) {
}

void Hires::onPreviewFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT("\nHires Preview callback at time %f; size %u\n",
              tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
}

void Hires::onVideoFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT("\nHires Video callback at time %f; size %u\n",
              tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
}

void Hires::onPictureFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT("\nHires Picture callback at time %f; size %u\n",
              tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);

  assert(0 == pthread_mutex_lock(&m_cameraFrameLock));

  m_frameReady = true;

  assert(0 == pthread_cond_signal(&m_cameraFrameReady));

  assert(0 == pthread_mutex_unlock(&m_cameraFrameLock));
}

void Hires::onMetadataFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT("\nHires Metadata callback at time %f; size %u\n",
              tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
}

// Private functions
int Hires::activate() {
  assert(m_cameraPtr != NULL);
  m_cameraPtr->addListener(this);
  return m_cameraPtr->startPreview();
}

void Hires::deactivate() {
  assert(m_cameraPtr != NULL);
  m_cameraPtr->removeListener(this);
  m_cameraPtr->stopPreview();
  return;
}
