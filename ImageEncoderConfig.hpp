#include <stdint.h>
#include "OMX_Core.h"
#include "OMX_Image.h"
#include "OMX_QCOMExtns.h"
#include "OMX_IndexExt.h"

#ifndef SDFLIGHT_CAMERAS_IMAGEENCODERCONFIG_HPP
#define SDFLIGHT_CAMERAS_IMAGEENCODERCONFIG_HPP

namespace ImageEncoderConfig {


enum {
  IMG_COMP_OMX_SPEC_VERSION = 0x00000101,
};

enum {
  IMG_COMP_PORT_INDEX_IN = 0,
  IMG_COMP_PORT_INDEX_OUT = 1,
  IMG_COMP_DEFAULT_WIDTH = 640,
  IMG_COMP_DEFAULT_HEIGHT = 480,
  IMG_COMP_IN_BUFFER_SIZE = IMG_COMP_DEFAULT_WIDTH * IMG_COMP_DEFAULT_HEIGHT * 3/2,
  IMG_COMP_OUT_BUFFER_SIZE = IMG_COMP_DEFAULT_WIDTH * IMG_COMP_DEFAULT_HEIGHT * 3/2
};

struct ImageEncoderConfigType
{
  ////////////////////////////////////////
  //======== Common static config
  OMX_IMAGE_CODINGTYPE codec;
  OMX_S32 inBufferCount;
  OMX_S32 outBufferCount;
  OMX_S32 inBufferSize;
  OMX_S32 outBufferSize;
};

OMX_ERRORTYPE Configure(OMX_HANDLETYPE encoder,
                        ImageEncoderConfigType& config);

} // end namespace ImageEncoderConfig

#endif // SDFLIGHT_CAMERAS_IMAGEENCODERCONFIG_HPP
