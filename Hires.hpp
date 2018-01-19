#include "camera.h"
#include "camera_parameters.h"
#include "Debug.hpp"

class Hires : private camera::ICameraListener
{
public:
  Hires();

private:
  virtual void onError() {}

  virtual void onControl(const camera::ControlEvent& control) {}

  virtual void onPreviewFrame(camera::ICameraFrame *frame) {}

  virtual void onVideoFrame(camera::ICameraFrame *frame) {}

  virtual void onPictureFrame(camera::ICameraFrame *frame) {}

  virtual void onMetadataFrame(camera::ICameraFrame *frame) {}

  camera::ICameraDevice* m_cameraPtr;

  camera::CameraParams m_params;
}; // class Hires
