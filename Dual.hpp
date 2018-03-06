#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#include "camera.h"
#include "camera_parameters.h"
#pragma GCC diagnostic pop

#include "Debug.hpp"

#ifndef SDFLIGHT_CAMERAS_DUAL_HPP
#define SDFLIGHT_CAMERAS_DUAL_HPP

class Dual
{
public:
  
  class Camera : public camera::ICameraListener {
  public:
    Camera(int cameraID,
	   const char* const cameraString,
	   bool print = true);

    ~Camera();

    camera::ICameraDevice* m_cameraPtr;
    
    camera::CameraParams m_params;
    
  private:
    // Interface functions
    virtual void onError();

    virtual void onControl(const camera::ControlEvent& control);

    virtual void onPreviewFrame(camera::ICameraFrame *frame);

    virtual void onVideoFrame(camera::ICameraFrame *frame);

    virtual void onPictureFrame(camera::ICameraFrame *frame);

    virtual void onMetadataFrame(camera::ICameraFrame *frame);

    const char* const m_cameraString;
    
    bool m_print;
    
    pthread_mutex_t m_cameraFrameLock;
  }; // class Camera
  
  Dual(bool print = true);

  ~Dual();

  int activateOptic();

  void deactivateOptic();

  int takeHiresPicture();
  
  int activateHires();

  void deactivateHires();
  
private:
  // Member variables
  bool m_print;

  Camera m_hires;
  
  Camera m_optic;
}; // class Dual

#endif // SDFLIGHT_CAMERAS_DUAL_HPP
