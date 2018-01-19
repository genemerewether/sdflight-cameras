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
  Hires();

  ~Hires();

  int activate();

  void deactivate();

  int takePicture();

  int startRecording();

  int stopRecording();

private:
  // Interface functions
  virtual void onError();

  virtual void onControl(const camera::ControlEvent& control);

  virtual void onPreviewFrame(camera::ICameraFrame *frame);

  virtual void onVideoFrame(camera::ICameraFrame *frame);

  virtual void onPictureFrame(camera::ICameraFrame *frame);

  virtual void onMetadataFrame(camera::ICameraFrame *frame);

  // Member variables
  camera::ICameraDevice* m_cameraPtr;

  camera::CameraParams m_params;

  bool m_frameReady;

  pthread_mutex_t m_cameraFrameLock;

  pthread_cond_t m_cameraFrameReady;
}; // class Hires
