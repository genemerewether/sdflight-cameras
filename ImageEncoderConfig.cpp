#include "ImageEncoderConfig.hpp"
#include <media/hardware/HardwareAPI.h>

#include "OMX_Core.h"
#include <assert.h>

namespace ImageEncoderConfig {

OMX_ERRORTYPE Configure(OMX_HANDLETYPE encoder,
                        ImageEncoderConfigType& config) {
  OMX_ERRORTYPE omxError;
  assert(encoder);

  // TODO(mereweth) - support other encoder types?

  // ----------------------------------------------------------------------
  // Input ports
  // ----------------------------------------------------------------------

  OMX_PARAM_PORTDEFINITIONTYPE inPortDef;
  inPortDef.nSize = sizeof(inPortDef);
  inPortDef.nPortIndex = (OMX_U32) IMG_COMP_PORT_INDEX_IN;
  omxError = OMX_GetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inPortDef);
  assert(omxError == OMX_ErrorNone);

  inPortDef.format.image.nFrameWidth = IMG_COMP_DEFAULT_WIDTH;
  inPortDef.format.image.nFrameHeight = IMG_COMP_DEFAULT_HEIGHT;
  inPortDef.nBufferCountActual = inPortDef.nBufferCountMin;
  config.inBufferCount = inPortDef.nBufferCountMin;

  omxError = OMX_SetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inPortDef);
  assert(omxError == OMX_ErrorNone);

  omxError = OMX_GetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inPortDef);
  assert(omxError == OMX_ErrorNone);
  config.inBufferSize = inPortDef.nBufferSize;

  // ----------------------------------------------------------------------
  // Output ports
  // ----------------------------------------------------------------------

  OMX_PARAM_PORTDEFINITIONTYPE outPortDef;
  outPortDef.nSize = sizeof(outPortDef);
  outPortDef.nPortIndex = (OMX_U32) IMG_COMP_PORT_INDEX_OUT;
  omxError = OMX_GetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &outPortDef);
  assert(omxError == OMX_ErrorNone);

  outPortDef.format.image.nFrameWidth = IMG_COMP_DEFAULT_WIDTH;
  outPortDef.format.image.nFrameHeight = IMG_COMP_DEFAULT_HEIGHT;
  outPortDef.nBufferCountActual = outPortDef.nBufferCountMin;
  config.outBufferCount = outPortDef.nBufferCountMin;

  omxError = OMX_SetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &outPortDef);
  assert(omxError == OMX_ErrorNone);

  omxError = OMX_GetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &outPortDef);
  assert(omxError == OMX_ErrorNone);
  config.outBufferSize = outPortDef.nBufferSize;

  return OMX_ErrorNone;
}


} // end namespace ImageEncoderConfig
