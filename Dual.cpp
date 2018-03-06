#include "Dual.hpp"
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#define DUAL_PCOLOR KMAG

#define DUAL_CAM_TYPE_OPTIC 1
#define DUAL_CAM_TYPE_HIRES 0

Dual::Dual(bool print) :
  m_print(print),
  m_hires(DUAL_CAM_TYPE_HIRES, "hires", print),
  m_optic(DUAL_CAM_TYPE_OPTIC, "optic", print)
{
  assert(m_hires.m_cameraPtr != NULL);
  m_hires.m_cameraPtr->addListener(&m_hires);
  
  assert(m_optic.m_cameraPtr != NULL);
  m_optic.m_cameraPtr->addListener(&m_optic);  
}

Dual::~Dual() {
}

// Public functions
int Dual::activateOptic() {
  assert(m_optic.m_cameraPtr != NULL);
  return m_optic.m_cameraPtr->startPreview();
}

void Dual::deactivateOptic() {
  assert(m_optic.m_cameraPtr != NULL);
  m_optic.m_cameraPtr->stopPreview();
  return;
}

int Dual::takeHiresPicture() {
  return m_hires.m_cameraPtr->takePicture();
}

int Dual::activateHires() {
  assert(m_hires.m_cameraPtr != NULL);
  int stat = 0; //m_hires.m_cameraPtr->startPreview();
  if (stat)
    return stat;
  else
    return m_hires.m_cameraPtr->startRecording();
}

void Dual::deactivateHires() {
  assert(m_hires.m_cameraPtr != NULL);
  m_hires.m_cameraPtr->stopRecording();
  //m_hires.m_cameraPtr->stopPreview();
  return;
}

Dual::Camera::Camera(int cameraType,
		     const char* const cameraString,
		     bool print) :
  m_cameraPtr(NULL),
  m_params(),
  m_cameraString(cameraString),
  m_print(print)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(DUAL_PCOLOR "\nDual::Camera constructor at time %f\n" KNRM,
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

    if (cameraInfo.func == cameraType)
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

Dual::Camera::~Camera() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(DUAL_PCOLOR "\nDual::Camera destructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  if(m_cameraPtr != NULL) {
    camera::ICameraDevice::deleteInstance(&m_cameraPtr);
    m_cameraPtr = NULL;
  }
  
  assert(0 == pthread_mutex_destroy(&m_cameraFrameLock));
}

// Interface functions
void Dual::Camera::onError() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(DUAL_PCOLOR "\nCamera %s Error callback at time %f\n" KNRM,
              m_cameraString, tv.tv_sec + tv.tv_usec / 1000000.0);
}

void Dual::Camera::onControl(const camera::ControlEvent& control) {
}

void Dual::Camera::onPreviewFrame(camera::ICameraFrame *frame) {
  if (m_print) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    DEBUG_PRINT(DUAL_PCOLOR "\nCamera %s Preview callback at time %f; size %u\n" KNRM,
                m_cameraString, tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
  }
}

void Dual::Camera::onVideoFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(DUAL_PCOLOR "\nCamera %s Video callback at time %f; size %u\n" KNRM,
              m_cameraString, tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
}

void Dual::Camera::onPictureFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(DUAL_PCOLOR "\nCamera %s Picture callback at time %f; size %u\n" KNRM,
              m_cameraString, tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
}

void Dual::Camera::onMetadataFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(DUAL_PCOLOR "\nCamera %s Metadata callback at time %f; size %u\n" KNRM,
              m_cameraString, tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
}
