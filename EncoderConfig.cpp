#include "EncoderConfig.hpp"
#include <media/hardware/HardwareAPI.h>

#include "OMX_Core.h"
#include <assert.h>

namespace EncoderConfig {

OMX_ERRORTYPE Configure(OMX_HANDLETYPE encoder,
                        EncoderConfigType& config) {
  OMX_ERRORTYPE omxError;
  assert(encoder);

  // TODO(mereweth) - support other encoder types?
  assert(config.codec == OMX_VIDEO_CodingAVC);

  OMX_VIDEO_PARAM_AVCTYPE avc;
  avc.nSize = sizeof(avc);
  avc.nPortIndex = (OMX_U32) IMG_COMP_PORT_INDEX_OUT; // output
  omxError = OMX_GetParameter(encoder,
                            OMX_IndexParamVideoAvc,
                            (OMX_PTR) &avc);
  assert(omxError == OMX_ErrorNone);

  // TODO(mereweth) - support other quality settings (# of B and P frames)?
  avc.nPFrames = config.intraPeriod - 1;
  avc.eProfile = OMX_VIDEO_AVCProfileBaseline;
  avc.nBFrames = 0;
  avc.bEntropyCodingCABAC = OMX_FALSE;

  avc.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterDisable;

  avc.eLevel = OMX_VIDEO_AVCLevel1;
  avc.bUseHadamard = OMX_FALSE;
  avc.nRefFrames = 1;
  avc.nRefIdx10ActiveMinus1 = 1;
  avc.nRefIdx11ActiveMinus1 = 0;
  avc.bEnableUEP = OMX_FALSE;
  avc.bEnableFMO = OMX_FALSE;
  avc.bEnableASO = OMX_FALSE;
  avc.bEnableRS = OMX_FALSE;
  avc.nAllowedPictureTypes = 2;
  avc.bFrameMBsOnly = OMX_FALSE;
  avc.bMBAFF = OMX_FALSE;
  avc.bWeightedPPrediction = OMX_FALSE;
  avc.nWeightedBipredicitonMode = 0;
  avc.bconstIpred = OMX_FALSE;
  avc.bDirect8x8Inference = OMX_FALSE;
  avc.bDirectSpatialTemporal = OMX_FALSE;
  avc.nCabacInitIdc = 0;

  omxError = OMX_SetParameter(encoder,
                              OMX_IndexParamVideoAvc,
                              (OMX_PTR) &avc);
  assert(omxError == OMX_ErrorNone);

  // TODO(mereweth) - IDR Period

  // TODO(mereweth) - Error correction

  // TODO(mereweth) - extraData

  // TODO(mereweth) - LTR encoding settings

  OMX_PARAM_PORTDEFINITIONTYPE inPortDef;
  inPortDef.nSize = sizeof(inPortDef);
  inPortDef.nPortIndex = (OMX_U32) IMG_COMP_PORT_INDEX_IN;
  omxError = OMX_GetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inPortDef);
  assert(omxError == OMX_ErrorNone);

  inPortDef.format.video.nFrameWidth = IMG_COMP_DEFAULT_WIDTH;
  inPortDef.format.video.nFrameHeight = IMG_COMP_DEFAULT_HEIGHT;
  inPortDef.nBufferCountActual = inPortDef.nBufferCountMin;

  omxError = OMX_SetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inPortDef);
  assert(omxError == OMX_ErrorNone);

  OMX_PARAM_PORTDEFINITIONTYPE outPortDef;
  outPortDef.nSize = sizeof(outPortDef);
  outPortDef.nPortIndex = (OMX_U32) IMG_COMP_PORT_INDEX_OUT;
  omxError = OMX_GetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &outPortDef);
  assert(omxError == OMX_ErrorNone);

  outPortDef.format.video.nFrameWidth = IMG_COMP_DEFAULT_WIDTH;
  outPortDef.format.video.nFrameHeight = IMG_COMP_DEFAULT_HEIGHT;
  outPortDef.nBufferCountActual = outPortDef.nBufferCountMin;
  outPortDef.format.video.xFramerate = config.framerate << 16;

  omxError = OMX_SetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &outPortDef);
  assert(omxError == OMX_ErrorNone);

  OMX_VIDEO_PARAM_BITRATETYPE bitrate;
  bitrate.nSize = sizeof(bitrate);
  bitrate.nPortIndex = (OMX_U32) IMG_COMP_PORT_INDEX_OUT;
  omxError = OMX_GetParameter(encoder,
                              OMX_IndexParamVideoBitrate,
                              (OMX_PTR) &bitrate);
  assert(omxError == OMX_ErrorNone);

  bitrate.eControlRate = config.controlRate;
  bitrate.nTargetBitrate = config.bitrate;

  omxError = OMX_SetParameter(encoder,
                              OMX_IndexParamVideoBitrate,
                              (OMX_PTR) &bitrate);
  assert(omxError == OMX_ErrorNone);

  return OMX_ErrorNone;
}


} // end namespace EncoderConfig
