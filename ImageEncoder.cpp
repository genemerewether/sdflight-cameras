#include "ImageEncoder.hpp"
#include "ImageEncoderConfig.hpp"
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include "Debug.hpp"

#include <media/hardware/HardwareAPI.h>

#include <qomx_core.h>

#define ENCODER_PCOLOR KCYN

ImageEncoder::ImageEncoder()
{
  OMX_ERRORTYPE omxError;
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nImageEncoder constructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  for (int i = 0; i < IMGENC_IMAGE_MODE_MAX; i++) {
    switch (i) {
      case IMGENC_IMG_13MP_RAW:
      case IMGENC_IMG_13MP:
      case IMGENC_IMG_13MP_HDR:
        m_inputs[i].dim.w = 4208;
        m_inputs[i].dim.h = 3120;
        break;
      case IMGENC_IMG_2MP:
      case IMGENC_IMG_2MP_HDR:
        m_inputs[i].dim.w = 1920;
        m_inputs[i].dim.h = 1080;
        break;
      case IMGENC_IMG_VGA:
      case IMGENC_IMG_VGA_HDR:
        m_inputs[i].dim.w = 640;
        m_inputs[i].dim.h = 480;
        break;
      default:
        DEBUG_PRINT(KRED "\nImageEncoder setup found invalid mode %d\n" KNRM, i);
        assert(0);
    }

    m_inputs[i].handle = jpeg_open(&m_inputs[i].ops, m_inputs[i].dim);
  }
}

ImageEncoder::~ImageEncoder()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nImageEncoder destructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

}
