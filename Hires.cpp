#include "Hires.hpp"
#include <assert.h>
#include <pthread.h>

#define HIRES_CAM_TYPE 0

Hires::Hires() :
  m_cameraPtr(NULL),
  m_params(),
  m_frameReady(false)
{
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
  if(m_cameraPtr != NULL) {
    camera::ICameraDevice::deleteInstance(&m_cameraPtr);
    m_cameraPtr = NULL;
  }

  assert(0 == pthread_mutex_destroy(&m_cameraFrameLock));

  assert(0 == pthread_cond_destroy(&m_cameraFrameReady));
}

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

int Hires::takePicture() {
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
  return 0;
}

// Interface functions
void Hires::onError() {
}

void Hires::onControl(const camera::ControlEvent& control) {
}

void Hires::onPreviewFrame(camera::ICameraFrame *frame) {
}

void Hires::onVideoFrame(camera::ICameraFrame *frame) {
}

void Hires::onPictureFrame(camera::ICameraFrame *frame) {
  assert(0 == pthread_mutex_lock(&m_cameraFrameLock));

  m_frameReady = true;

  assert(0 == pthread_cond_signal(&m_cameraFrameReady));

  assert(0 == pthread_mutex_unlock(&m_cameraFrameLock));
}

void Hires::onMetadataFrame(camera::ICameraFrame *frame) {
}
