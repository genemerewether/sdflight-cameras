#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#include "camera.h"
#include "camera_parameters.h"
#pragma GCC diagnostic pop

#include "Debug.hpp"

class Hires : private camera::ICameraListener
{
public:
  typedef enum {
    HIRES_IMG_13MP_RAW,
    HIRES_IMG_13MP,
    HIRES_IMG_13MP_HDR,
    HIRES_IMG_2MP,
    HIRES_IMG_2MP_HDR,
    HIRES_IMG_QVGA,
    HIRES_IMG_QVGA_HDR,
    HIRES_IMAGE_MODE_MAX
  } HiresImageMode;

  typedef enum {
    HIRES_VID_4K,
    HIRES_VID_4K_HDR,
    HIRES_VID_1080P,
    HIRES_VID_1080P_HDR,
    HIRES_VIDEO_MODE_MAX
  } HiresVideoMode;

  Hires(bool save = false);

  ~Hires();

  int takePicture(HiresImageMode mode = Hires::HIRES_IMG_13MP);

  int startRecording(HiresVideoMode mode = Hires::HIRES_VID_4K);

  void stopRecording();

private:
  // Interface functions
  virtual void onError();

  virtual void onControl(const camera::ControlEvent& control);

  virtual void onPreviewFrame(camera::ICameraFrame *frame);

  virtual void onVideoFrame(camera::ICameraFrame *frame);

  virtual void onPictureFrame(camera::ICameraFrame *frame);

  virtual void onMetadataFrame(camera::ICameraFrame *frame);

  // Private functions
  int activate();

  void deactivate();

  // Member variables
  camera::ICameraDevice* m_cameraPtr;

  camera::CameraParams m_params;

  bool m_frameReady;

  bool m_recording;
  
  bool m_save;

  HiresImageMode m_imageMode;

  HiresVideoMode m_videoMode;

  pthread_mutex_t m_cameraFrameLock;

  pthread_cond_t m_cameraFrameReady;

}; // class Hires
