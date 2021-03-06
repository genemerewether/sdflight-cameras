#include "Hires.hpp"
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define HIRES_PCOLOR KBLU

#define HIRES_CAM_TYPE 0

#define HIRES_IMG_WAIT_SEC 3
#define HIRES_AUTOFOCUS_WAIT_SEC 1

Hires::Hires(bool alwaysCycle, bool save) :
  m_cameraPtr(NULL),
  m_params(),
  m_pictureReady(false),
  m_alwaysCycle(alwaysCycle),
  m_doAutofocus(false),
  m_autofocusDone(false),
  m_videoReady(false),
  m_recording(false),
  m_save(save),
  m_framesAcquired(0u),
  m_frameStopRecording(-1),
  m_imageMode(HIRES_IMAGE_MODE_MAX),
  m_videoMode(HIRES_VIDEO_MODE_MAX),
  m_pictureOutBuffer(NULL)/*,
  m_encoder()*/
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(HIRES_PCOLOR "\nHires constructor at time %f\n" KNRM,
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

  stat = m_params.init(m_cameraPtr);
  assert(stat == 0);

  m_params.set("dis", "eis_2_0");
  m_params.setFocusMode("continuous-video");

  camera::ImageSize imageSize(320, 240);
  m_params.setPreviewSize(imageSize);

  stat = m_params.commit();
  assert(stat == 0);

  m_cameraPtr->addListener(this);

  assert(0 == pthread_mutex_init(&m_cameraFrameLock, 0));
  assert(0 == pthread_cond_init(&m_cameraFrameReady, 0));

  assert(0 == pthread_mutex_init(&m_autofocusLock, 0));
  assert(0 == pthread_cond_init(&m_autofocusReady, 0));

  if (m_alwaysCycle) {
    stat = this->activate();
    assert(stat == 0);
  }
}

Hires::~Hires() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(HIRES_PCOLOR "\nHires destructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  m_cameraPtr->removeListener(this);

  if(m_cameraPtr != NULL) {
    camera::ICameraDevice::deleteInstance(&m_cameraPtr);
    m_cameraPtr = NULL;
  }

  assert(0 == pthread_mutex_destroy(&m_cameraFrameLock));
  assert(0 == pthread_cond_destroy(&m_cameraFrameReady));

  assert(0 == pthread_mutex_destroy(&m_autofocusLock));
  assert(0 == pthread_cond_destroy(&m_autofocusReady));
}

void Hires::flowAutoStop() {
  while (1) {
    if ((m_frameStopRecording >= 0) &&
        ((int) m_framesAcquired > m_frameStopRecording)) {
      this->stopRecording();
      DEBUG_PRINT(HIRES_PCOLOR "\nHires Video auto-deactivating; %u of %d acquired\n" KNRM,
                  m_framesAcquired,
                  m_frameStopRecording+1);
      break;
    }
    sleep(1);
  }
}

void Hires::recordingAutoStop() {
  while (1) {
    if ((m_frameStopRecording >= 0) &&
        ((int) m_framesAcquired > m_frameStopRecording)) {
      this->stopRecording();
      DEBUG_PRINT(HIRES_PCOLOR "\nHires Video auto-deactivating; %u of %d acquired\n" KNRM,
                  m_framesAcquired,
                  m_frameStopRecording+1);
      break;
    }
    sleep(1);
  }
}

void Hires::setPictureOutBuffer(buffer_t* outBuffer) {
  assert(0 == pthread_mutex_lock(&m_cameraFrameLock));
  m_pictureOutBuffer = outBuffer;
  assert(0 == pthread_mutex_unlock(&m_cameraFrameLock));
}

int Hires::takePicture(HiresImageMode mode) {
  int stat = -1;
  struct timeval tv;
  gettimeofday(&tv,NULL);
  if (m_recording) {
    DEBUG_PRINT(HIRES_PCOLOR "\nHires takePicture called while recording at time %f\n" KNRM,
                tv.tv_sec + tv.tv_usec / 1000000.0);
    return -1;
  }
  DEBUG_PRINT(HIRES_PCOLOR "\nHires takePicture called at time %f, mode %d\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0, mode);

  m_imageMode = mode;

  if ((mode == HIRES_IMG_13MP_HDR) ||
      (mode == HIRES_IMG_2MP_HDR) ||
      (mode == HIRES_IMG_VGA_HDR)) {
    m_params.set("scene-mode", "hdr");
    m_params.set("hdr-need-1x", "true");
    //m_params.set("zsl", "on");
  }

  if (mode == HIRES_IMG_13MP_RAW) {
    m_params.set("raw-size", "4208x3120");
    m_params.set("preview-format", "bayer-rggb");
    m_params.set("picture-format", "bayer-mipi-10bggr");
  }
  else if (mode == HIRES_IMG_JPG_TEST) {
    m_params.set("picture-format", "jpeg");
    m_params.set("preview-format", "yuv420sp");
  }
  else {
    m_params.set("preview-format", "yuv420sp");
    m_params.set("picture-format", "yuv420sp");
  }

  camera::ImageSize imageSize;
  switch (mode) {
    case HIRES_IMG_13MP_RAW:
    case HIRES_IMG_13MP:
    case HIRES_IMG_13MP_HDR:
      imageSize = camera::ImageSize(4208, 3120);
      break;
    case HIRES_IMG_2MP:
    case HIRES_IMG_2MP_HDR:
      imageSize = camera::ImageSize(1920, 1080);
      break;
    case HIRES_IMG_VGA:
    case HIRES_IMG_VGA_HDR:
      imageSize = camera::ImageSize(640, 480);
      break;
    case HIRES_IMG_JPG_TEST:
      //imageSize = camera::ImageSize(3840, 2160);
      imageSize = camera::ImageSize(1920, 1080);
      //imageSize = camera::ImageSize(640, 480);
      break;
    default:
      DEBUG_PRINT(KRED "\nHires takePicture called with invalid mode %d\n" KNRM, mode);
      assert(0);
  }

  m_params.setPictureSize(imageSize);
  imageSize = camera::ImageSize(320, 240);
  m_params.setPreviewSize(imageSize);
  stat = m_params.commit();
  assert(stat == 0);

  const camera::ImageSize getSize = m_params.getPictureSize();
  const camera::ImageSize getPreviewSize = m_params.getPreviewSize();
  DEBUG_PRINT(HIRES_PCOLOR "\nHires has raw size %s; width %d; height %d\n; preview width %d; preview height %d\n" KNRM,
              m_params.get("raw-size").c_str(),
              getSize.width,
              getSize.height,
              getPreviewSize.width,
              getPreviewSize.height);

  if (!m_alwaysCycle)
  {
    stat = this->activate();
    if (stat) {
      return stat;
    }
  }

  struct timeval now;
  struct timespec wait;
  if (m_doAutofocus) {
    stat = m_cameraPtr->startAutoFocus();
    if (stat) {
      printf("Autofocus start failed; stat %d\n", stat);
      return stat;
    }
    assert(0 == pthread_mutex_lock(&m_autofocusLock));
    m_autofocusDone = false;

    gettimeofday(&now,NULL);
    wait.tv_sec = static_cast<unsigned int>(HIRES_AUTOFOCUS_WAIT_SEC) +
      now.tv_sec;
    wait.tv_nsec = now.tv_usec * 1000;

    stat = 0;
    while (!m_autofocusDone && stat == 0) {
      stat = pthread_cond_timedwait(&m_autofocusReady, &m_autofocusLock,
				    &wait);
    }
    if (stat == ETIMEDOUT) {
      DEBUG_PRINT(KRED "Hires autofocus timed out as expected in takePicture\n" KNRM);
      //assert(0 == pthread_mutex_unlock(&m_autofocusLock));
      //this->deactivate();
    }

    assert(0 == pthread_mutex_unlock(&m_autofocusLock));
  }

  assert(0 == pthread_mutex_lock(&m_cameraFrameLock));
  m_pictureReady = false;

  stat = m_cameraPtr->takePicture();
  if (stat) {
    assert(0 == pthread_mutex_unlock(&m_cameraFrameLock));
    this->deactivate();
    return stat;
  }

  gettimeofday(&now,NULL);
  wait.tv_sec = static_cast<unsigned int>(HIRES_IMG_WAIT_SEC) +
    now.tv_sec;
  wait.tv_nsec = now.tv_usec * 1000;

  stat = 0;
  while (!m_pictureReady && stat == 0) {
    stat = pthread_cond_timedwait(&m_cameraFrameReady, &m_cameraFrameLock,
                                  &wait);
  }
  if (stat == ETIMEDOUT) {
    DEBUG_PRINT(KRED "Hires timed out in takePicture\n" KNRM);
  }

  assert(0 == pthread_mutex_unlock(&m_cameraFrameLock));

  if (!m_alwaysCycle)
  {
    this->deactivate();
  }

  return 0;
}

int Hires::startRecording(HiresVideoMode mode,
                          int frames) {
  int stat = -1;
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(HIRES_PCOLOR "\nHires startRecording called at time %f, mode %d\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0, mode);

  if ((mode == HIRES_VID_MAX_HDR)   ||
      (mode == HIRES_VID_1080P_HDR) ||
      (mode == HIRES_VID_720P_HDR)  ||
      (mode == HIRES_VID_480P_HDR)) {
    m_params.set("scene-mode", "hdr");
    m_params.set("hdr-need-1x", "true");
    //m_params.set("zsl", "on");
  }

  m_params.set("preview-format", "yuv420sp");
  //m_params.set("picture-format", "yuv420sp");

  camera::ImageSize imageSize;
  switch (mode) {
    case HIRES_VID_4K:
      imageSize = camera::ImageSize(4208, 3120);
      break;
    case HIRES_VID_MAX_HDR:
      imageSize = camera::ImageSize(3840, 2160);//(2104, 1560);
      break;
    case HIRES_VID_1080P:
    case HIRES_VID_1080P_HDR:
      imageSize = camera::ImageSize(1920, 1080);
      break;
    case HIRES_VID_720P:
    case HIRES_VID_720P_HDR:
      imageSize = camera::ImageSize(1280, 720);
      break;
    case HIRES_VID_480P:
    case HIRES_VID_480P_HDR:
      imageSize = camera::ImageSize(640, 480);
      break;
    default:
      DEBUG_PRINT(KRED "\nHires startRecording called with invalid mode %d\n" KNRM, mode);
      assert(0);
  }

  m_params.setVideoSize(imageSize);

  // TODO(mereweth) - go smaller?
  imageSize = camera::ImageSize(320, 240);
  m_params.setPreviewSize(imageSize);

  stat = m_params.commit();
  assert(stat == 0);

  const camera::ImageSize getSize = m_params.getVideoSize();
  DEBUG_PRINT(HIRES_PCOLOR "\nHires has raw size %s; width %d; height %d\n" KNRM,
              m_params.get("raw-size").c_str(),
              getSize.width,
              getSize.height);

  m_videoMode = mode;

  assert(0 == pthread_mutex_lock(&m_cameraFrameLock));
  m_videoReady = false;
  m_recording = true;
  m_framesAcquired = 0;
  m_frameStopRecording = frames - 1; // zero-index

  if (!m_alwaysCycle)
  {
    stat = this->activate();
    if (stat) {
      assert(0 == pthread_mutex_unlock(&m_cameraFrameLock));
      return stat;
    }
  }

  stat = m_cameraPtr->startRecording();
  if (stat) {
    assert(0 == pthread_mutex_unlock(&m_cameraFrameLock));

    if (!m_alwaysCycle)
    {
      this->deactivate();
    }

    return stat;
  }

  struct timeval now;
  gettimeofday(&now,NULL);
  struct timespec wait;
  wait.tv_sec = static_cast<unsigned int>(HIRES_IMG_WAIT_SEC) +
    now.tv_sec;
  wait.tv_nsec = now.tv_usec * 1000;

  stat = 0;
  while (!m_videoReady && stat == 0) {
    stat = pthread_cond_timedwait(&m_cameraFrameReady, &m_cameraFrameLock,
                                  &wait);
  }
  if (stat == ETIMEDOUT) {
    DEBUG_PRINT(KRED "Hires timed out in startRecording\n" KNRM);
  }

  assert(0 == pthread_mutex_unlock(&m_cameraFrameLock));

  gettimeofday(&tv,NULL);
  DEBUG_PRINT(HIRES_PCOLOR "\nHires Video first callback at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  return 0;
}

void Hires::stopRecording() {
  m_cameraPtr->stopRecording();
  m_recording = false;

  if (!m_alwaysCycle)
  {
    this->deactivate();
  }
}

// Interface functions
void Hires::onError() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(HIRES_PCOLOR "\nHires Error callback at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
}

void Hires::onControl(const camera::ControlEvent& control) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(HIRES_PCOLOR "\nHires Control callback type %d at time %f\n" KNRM,
              control.type,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  if (control.type == camera::CAMERA_EVT_FOCUS) {
      if (control.ext1 == true) {
          printf("Hires Control callback auto focus done\n");

          assert(0 == pthread_mutex_lock(&m_autofocusLock));
          m_autofocusDone = true;
          assert(0 == pthread_cond_signal(&m_autofocusReady));
          assert(0 == pthread_mutex_unlock(&m_autofocusLock));
      } else {
          printf("Hires Control callback auto focus failed\n");
          m_autofocusDone = false;
      }
  }
}

void Hires::onPreviewFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);

  if (!m_pictureReady) {// we are waiting for a picture
    DEBUG_PRINT(HIRES_PCOLOR "\nHires Preview callback size %u while waiting for picture at time %f\n" KNRM,
                frame->size, tv.tv_sec + tv.tv_usec / 1000000.0);
  }

  if ((m_frameStopRecording >= 0) &&
      ((int) m_framesAcquired > m_frameStopRecording)) {
    DEBUG_PRINT(HIRES_PCOLOR "\nHires Preview callback size %u while deactivating at time %f\n" KNRM,
                frame->size, tv.tv_sec + tv.tv_usec / 1000000.0);
    return;
  }
}

void Hires::onVideoFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);

  if ((m_frameStopRecording >= 0) &&
      ((int) m_framesAcquired > m_frameStopRecording)) {
    /*DEBUG_PRINT(HIRES_PCOLOR "\nHires Video callback while deactivating at time %f\n" KNRM,
                tv.tv_sec + tv.tv_usec / 1000000.0);*/
    return;
  }

  DEBUG_PRINT(HIRES_PCOLOR "\nHires Video callback at time %f; size %u\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);

  m_framesAcquired++;

  if (m_save) {
    const char* fileName;
    switch (m_videoMode) {
      case HIRES_VID_4K:
        fileName = "HIRES_VID_4K";
        break;
      case HIRES_VID_MAX_HDR:
        fileName = "HIRES_VID_MAX_HDR";
        break;
      case HIRES_VID_1080P:
        fileName = "HIRES_VID_1080P";
        break;
      case HIRES_VID_1080P_HDR:
        fileName = "HIRES_VID_1080P_HDR";
        break;
      case HIRES_VID_720P:
        fileName = "HIRES_VID_720P";
        break;
      case HIRES_VID_720P_HDR:
        fileName = "HIRES_VID_720P_HDR";
        break;
      case HIRES_VID_480P:
        fileName = "HIRES_VID_480P";
        break;
      case HIRES_VID_480P_HDR:
        fileName = "HIRES_VID_480P_HDR";
        break;
      default:
        DEBUG_PRINT(KRED "\nHires invalid video mode %d in onVideoFrame\n" KNRM, m_videoMode);
        fileName = "HIRES_VID_UNKNOWN";
    }
    int fid = open(fileName,
                   O_CREAT | O_WRONLY | O_APPEND,
                   S_IRUSR | S_IWUSR |  S_IRGRP | S_IWGRP);
    assert(fid != -1);

    write(fid, frame->data, frame->size);

    assert(close(fid) != -1);
  }

  assert(0 == pthread_mutex_lock(&m_cameraFrameLock));

  m_videoReady = true;

  assert(0 == pthread_cond_signal(&m_cameraFrameReady));

  assert(0 == pthread_mutex_unlock(&m_cameraFrameLock));
}

void Hires::onPictureFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(HIRES_PCOLOR "\nHires Picture callback at time %f; size %u\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);

  if (m_save) {
    const char* fileName;
    switch (m_imageMode) {
      case HIRES_IMG_13MP_RAW:
        fileName = "HIRES_IMG_13MP_RAW";
        break;
      case HIRES_IMG_13MP:
        fileName = "HIRES_IMG_13MP";
        break;
      case HIRES_IMG_13MP_HDR:
        fileName = "HIRES_IMG_13MP_HDR";
        break;
      case HIRES_IMG_2MP:
        fileName = "HIRES_IMG_2MP";
        break;
      case HIRES_IMG_2MP_HDR:
        fileName = "HIRES_IMG_2MP_HDR";
        break;
      case HIRES_IMG_VGA:
        fileName = "HIRES_IMG_VGA";
        break;
      case HIRES_IMG_VGA_HDR:
        fileName = "HIRES_IMG_VGA_HDR";
        break;
      case HIRES_IMG_JPG_TEST:
        fileName = "HIRES_IMG_JPG_TEST";
        break;
      default:
        DEBUG_PRINT(KRED "\nHires invalid image mode %d in onPictureFrame\n" KNRM, m_imageMode);
        fileName = "HIRES_IMG_UNKNOWN";
    }
    int fid = open(fileName,
                   O_CREAT | O_WRONLY | O_TRUNC,
                   S_IRUSR | S_IWUSR |  S_IRGRP | S_IWGRP);
    assert(fid != -1);

    write(fid, frame->data, frame->size);

    assert(close(fid) != -1);
  }

  // TODO(merewet) - use a second lock?
  assert(0 == pthread_mutex_lock(&m_cameraFrameLock));

  if (m_pictureOutBuffer == NULL) {
    DEBUG_PRINT(HIRES_PCOLOR "\nHires picture out buffer NULL in onPictureFrame\n" KNRM);
  }
  else {
    if (m_pictureOutBuffer->addr == NULL) {
      DEBUG_PRINT(KRED "\nHires picture out buffer addr NULL in onPictureFrame\n" KNRM);
      assert(0);
    }
    int size = frame->size;
    if (m_pictureOutBuffer->size < size) {
      DEBUG_PRINT(KRED "\nHires picture out buffer too small, %d vs %d, in onPictureFrame\n" KNRM,
                  m_pictureOutBuffer->size, size);
      size = m_pictureOutBuffer->size;
    }
    (void) memcpy((void*) m_pictureOutBuffer->addr, frame->data, size);
  }

  m_pictureReady = true;

  assert(0 == pthread_cond_signal(&m_cameraFrameReady));

  assert(0 == pthread_mutex_unlock(&m_cameraFrameLock));
}

void Hires::onMetadataFrame(camera::ICameraFrame *frame) {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(HIRES_PCOLOR "\nHires Metadata callback at time %f; size %u\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0, frame->size);
}

// Private functions
int Hires::activate() {
  assert(m_cameraPtr != NULL);
  //m_cameraPtr->addListener(this);
  int stat = m_cameraPtr->startPreview();
  m_cameraPtr->sendFaceDetectCommand(false);
  return stat;
}

void Hires::deactivate() {
  assert(m_cameraPtr != NULL);
  m_cameraPtr->stopPreview();
  //m_cameraPtr->removeListener(this);
  return;
}
