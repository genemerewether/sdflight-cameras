#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#include "camera.h"
#include "camera_parameters.h"
#pragma GCC diagnostic pop

#include "Debug.hpp"
#include "ImageEncoder.hpp"

#ifndef SDFLIGHT_CAMERAS_OPTIC_HPP
#define SDFLIGHT_CAMERAS_OPTIC_HPP

class Optic : private camera::ICameraListener
{
public:
  Optic(bool print = true);

  ~Optic();

  int activate();

  void deactivate();

private:
  // Interface functions
  virtual void onError();

  virtual void onControl(const camera::ControlEvent& control);

  virtual void onPreviewFrame(camera::ICameraFrame *frame);

  virtual void onVideoFrame(camera::ICameraFrame *frame);

  virtual void onPictureFrame(camera::ICameraFrame *frame);

  virtual void onMetadataFrame(camera::ICameraFrame *frame);

  // Member variables
  bool m_print;

  camera::ICameraDevice* m_cameraPtr;

  camera::CameraParams m_params;

  ImageEncoder m_imageEncoder;

  pthread_mutex_t m_cameraFrameLock;
}; // class Optic

#endif // SDFLIGHT_CAMERAS_OPTIC_HPP
