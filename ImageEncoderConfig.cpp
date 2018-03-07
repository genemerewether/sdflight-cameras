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
  // Input port
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
  inPortDef.format.image.nStride = IMG_COMP_DEFAULT_STRIDE;
  //inPortDef.format.image.nSliceHeight
  //inPortDef.format.image.eColorFormat
  //inPortDef.format.image.nBufferSize
  inPortDef.nBufferCountActual = inPortDef.nBufferCountMin;
  config.inBufferCount = inPortDef.nBufferCountMin;

  omxError = OMX_SetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inPortDef);
  assert(omxError == OMX_ErrorNone);

//   omxError = OMX_GetParameter(encoder,
//                               OMX_IndexParamPortDefinition,
//                               (OMX_PTR) &inPortDef);
//   assert(omxError == OMX_ErrorNone);
//   config.inBufferSize = inPortDef.nBufferSize;

  // ----------------------------------------------------------------------
  // Thumbnail port
  // ----------------------------------------------------------------------

  OMX_PARAM_PORTDEFINITIONTYPE inThumbPortDef;
  inThumbPortDef.nSize = sizeof(inThumbPortDef);
  inThumbPortDef.nPortIndex = (OMX_U32) IMG_COMP_PORT_INDEX_IN;
  omxError = OMX_GetParameter(encoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inThumbPortDef);
  assert(omxError == OMX_ErrorNone);

  OMX_SendCommand(encoder, OMX_CommandPortDisable,
                  IMG_COMP_PORT_INDEX_THUMB, NULL);
  assert(omxError == OMX_ErrorNone);

//   inThumbPortDef.format.image.nFrameWidth = IMG_COMP_DEFAULT_WIDTH;
//   inThumbPortDef.format.image.nFrameHeight = IMG_COMP_DEFAULT_HEIGHT;
//   inThumbPortDef.format.image.nStride = IMG_COMP_DEFAULT_STRIDE;
//   //inThumbPortDef.format.image.nSliceHeight
//   //inThumbPortDef.format.image.eColorFormat
//   //inThumbPortDef.format.image.nBufferSize
//   inThumbPortDef.nBufferCountActual = inThumbPortDef.nBufferCountMin;
//   config.inBufferCount = inThumbPortDef.nBufferCountMin;

//   omxError = OMX_SetParameter(encoder,
//                               OMX_IndexParamPortDefinition,
//                               (OMX_PTR) &inThumbPortDef);
//   assert(omxError == OMX_ErrorNone);

//   omxError = OMX_GetParameter(encoder,
//                               OMX_IndexParamPortDefinition,
//                               (OMX_PTR) &inThumbPortDef);
//   assert(omxError == OMX_ErrorNone);
//   config.inBufferSize = inThumbPortDef.nBufferSize;

  // ----------------------------------------------------------------------
  // Output port
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
