#include "ImageEncoderConfig.hpp"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "mm_jpeg_interface.h"
#include "mm_jpeg_ionbuf.h"
#ifdef __cplusplus
}
#endif

#ifndef SDFLIGHT_CAMERAS_IMAGEENCODER_HPP
#define SDFLIGHT_CAMERAS_IMAGEENCODER_HPP

class ImageEncoder
{
public:
  typedef enum {
    IMGENC_IMG_13MP,
    IMGENC_IMG_2MP,
    IMGENC_IMG_VGA,
    IMGENC_IMAGE_MODE_MAX
  } ImageEncoderInputType;

  ImageEncoder();

  ~ImageEncoder();

private:
  typedef struct {
    uint32_t handle;
    mm_jpeg_ops_t ops;
    mm_jpeg_job_t job;
    mm_jpeg_encode_params_t params;
    mm_dimension dim;
    buffer_t input;
    buffer_t output;
  } ImageEncoderInterface;

  ImageEncoderInterface m_inputs[IMGENC_IMAGE_MODE_MAX];

}; // class ImageEncoder

#endif // SDFLIGHT_CAMERAS_IMAGEENCODER_HPP
