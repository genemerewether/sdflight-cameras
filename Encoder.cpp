#include "Encoder.hpp"
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include "Debug.hpp"

#define ENCODER_PCOLOR KCYN

Encoder::Encoder()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nEncoder constructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  OMX_ERRORTYPE omxError;
  OMX_CALLBACKTYPE callbacks = {
    .EventHandler = eventCallback,
    .EmptyBufferDone = emptyDoneCallback,
    .FillBufferDone = fillDoneCallback,
  };

  omxError = OMX_GetHandle(&m_omxEncoder,
                           (OMX_STRING)"OMX.qcom.video.encoder.avc",
                           this, &callbacks);
  if (OMX_ErrorNone != omxError) {
    DEBUG_PRINT("Failed to find video.encoder.h263: 0x%x", omxError);
    assert(0);
  }
}

Encoder::~Encoder()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nEncoder destructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  OMX_ERRORTYPE omxError;

  omxError = OMX_SendCommand(m_omxEncoder, OMX_CommandStateSet,
                             (OMX_U32)OMX_StateIdle, NULL);
  assert(OMX_ErrorNone == omxError);

  // TODO(mereweth)
  // wait for EventHandler with OMX_EventCmdComplete, OMX_StateIdle

  omxError = OMX_SendCommand(m_omxEncoder, OMX_CommandStateSet,
                             (OMX_U32)OMX_StateLoaded, NULL);
  assert(OMX_ErrorNone == omxError);

  // TODO(mereweth)
  // free all input and output buffers

  // TODO(mereweth)
  // wait for EventHandler with OMX_EventCmdComplete, OMX_StateLoaded

  omxError = OMX_FreeHandle(m_omxEncoder);
  assert(OMX_ErrorNone == omxError);
}

OMX_ERRORTYPE Encoder::eventCallback(OMX_IN OMX_HANDLETYPE compHandle,
                                     OMX_IN OMX_PTR context,
                                     OMX_IN OMX_EVENTTYPE event,
                                     OMX_IN OMX_U32 data1,
                                     OMX_IN OMX_U32 data2,
                                     OMX_IN OMX_PTR eventData)
{
  Encoder* comp = static_cast<Encoder*>(context);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE Encoder::emptyDoneCallback(OMX_IN OMX_HANDLETYPE compHandle,
                                         OMX_IN OMX_PTR context,
                                         OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  Encoder* comp = static_cast<Encoder*>(context);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE Encoder::fillDoneCallback(OMX_IN OMX_HANDLETYPE compHandle,
                                        OMX_IN OMX_PTR context,
                                        OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  Encoder* comp = static_cast<Encoder*>(context);
  return OMX_ErrorNone;
}
