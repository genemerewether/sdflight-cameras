#include <stdint.h>
#include "OMX_Core.h"
#include "OMX_Video.h"
#include "OMX_QCOMExtns.h"
#include "OMX_IndexExt.h"
#include "OMX_VideoExt.h"

#ifndef SDFLIGHT_CAMERAS_ENCODER_CONFIG_HPP
#define SDFLIGHT_CAMERAS_ENCODER_CONFIG_HPP

namespace EncoderConfig {


enum {
  IMG_COMP_OMX_SPEC_VERSION = 0x00000101,
};

enum {
  IMG_COMP_PORT_INDEX_IN = 0,
  IMG_COMP_PORT_INDEX_OUT = 1,
  IMG_COMP_DEFAULT_BITRATE = 5000000, // 5 Mbps
  IMG_COMP_DEFAULT_FRAMERATE = 30,
  IMG_COMP_DEFAULT_WIDTH = 4208,
  IMG_COMP_DEFAULT_HEIGHT = 3120
};

struct EncoderConfigType
{
  ////////////////////////////////////////
  //======== Common static config
  OMX_VIDEO_CODINGTYPE codec;
  OMX_VIDEO_CONTROLRATETYPE controlRate;
  OMX_S32 resyncMarkerSpacing;
  OMX_S32 intraRefreshMBCount;
  OMX_S32 frameWidth;
  OMX_S32 frameHeight;
  OMX_S32 outputFrameWidth;
  OMX_S32 outputFrameHeight;
  OMX_S32 bitrate;
  OMX_S32 framerate;
  OMX_S32 rotation;
  OMX_S32 intraPeriod;
  OMX_S32 minQp;
  OMX_S32 maxQp;
  OMX_BOOL extraData;
  //======== LTR encoding config
  OMX_S32 LTRMode;
  OMX_S32 LTRCount;
  OMX_S32 LTRPeriod;
};

OMX_ERRORTYPE Configure(OMX_HANDLETYPE encoder,
                        EncoderConfigType& config);

} // end namespace EncoderConfig

#endif // SDFLIGHT_CAMERAS_ENCODER_CONFIG_HPP