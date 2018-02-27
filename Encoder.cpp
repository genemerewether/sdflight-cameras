#include "Encoder.hpp"
#include "EncoderConfig.hpp"
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include "Debug.hpp"

#include <media/hardware/HardwareAPI.h>

#define ENCODER_PCOLOR KCYN

Encoder::Encoder() :
  m_omxEncoder(NULL),
  m_omxEncoderState(OMX_StateInvalid),
  m_omxInputBuffers(NULL),
  m_omxOutputBuffers(NULL)
{
  OMX_ERRORTYPE omxError;
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nEncoder constructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  assert(0 == pthread_mutex_init(&m_omxEncoderStateLock, 0));

  assert(0 == pthread_cond_init(&m_omxEncoderStateChange, 0));

  omxError = OMX_Init();
  assert(OMX_ErrorNone == omxError);

  OMX_CALLBACKTYPE callbacks = {
    .EventHandler = eventCallback,
    .EmptyBufferDone = emptyDoneCallback,
    .FillBufferDone = fillDoneCallback,
  };

  omxError = OMX_GetHandle(&m_omxEncoder,
                           (OMX_STRING)"OMX.qcom.video.encoder.avc",
                           this, &callbacks);
  if (OMX_ErrorNone != omxError) {
    DEBUG_PRINT("Failed to find video.encoder.avc: 0x%x", omxError);
    assert(0);
  }

  {   // extension "OMX.google.android.index.storeMetaDataInBuffers"
    android::StoreMetaDataInBuffersParams meta_mode_param = {
      .nSize = sizeof(android::StoreMetaDataInBuffersParams),
      .nVersion = EncoderConfig::IMG_COMP_OMX_SPEC_VERSION,
      .nPortIndex = EncoderConfig::IMG_COMP_PORT_INDEX_IN,
      .bStoreMetaData = OMX_TRUE,
    };
    omxError = OMX_SetParameter(m_omxEncoder,
                                (OMX_INDEXTYPE) OMX_QcomIndexParamVideoMetaBufferMode,
                                (OMX_PTR) &meta_mode_param);
  }
  if (OMX_ErrorNone != omxError) {
    DEBUG_PRINT(ENCODER_PCOLOR "Failed to enable Meta data mode: 0x%x" KNRM,
                omxError);
    assert(0);
  }

  EncoderConfig::EncoderConfigType config = {
      .codec = OMX_VIDEO_CodingAVC,
      .controlRate = OMX_Video_ControlRateVariable,
      .resyncMarkerSpacing = 0,
      .intraRefreshMBCount = 0,
      .bitrate = EncoderConfig::IMG_COMP_DEFAULT_BITRATE,
      .framerate = EncoderConfig::IMG_COMP_DEFAULT_FRAMERATE << 16,
      .rotation = 0,
      .inBufferCount = 0,
      .outBufferCount = 0,
      .inBufferSize = 0,
      .outBufferSize = 0,
      .intraPeriod = EncoderConfig::IMG_COMP_DEFAULT_FRAMERATE * 2,
      .minQp = 2,
      .maxQp = 31,
      .extraData = OMX_FALSE,
      .LTRMode = 0,
      .LTRCount = 0,
      .LTRPeriod = 0
  };

  omxError = EncoderConfig::Configure(m_omxEncoder, config);
  assert(OMX_ErrorNone == omxError);

  omxError = OMX_GetState(m_omxEncoder, &m_omxEncoderState);
  assert(OMX_ErrorNone == omxError);
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nOMX state %d at time %f\n" KNRM,
              m_omxEncoderState,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  omxError = OMX_SendCommand(m_omxEncoder, OMX_CommandStateSet,
                             (OMX_U32)OMX_StateIdle, NULL);
  assert(OMX_ErrorNone == omxError);

  m_omxInputBuffers = (OMX_BUFFERHEADERTYPE**)calloc(
      config.inBufferCount, sizeof(OMX_BUFFERHEADERTYPE*));
  assert(m_omxInputBuffers != NULL);

  m_omxOutputBuffers = (OMX_BUFFERHEADERTYPE**)calloc(
      config.outBufferCount, sizeof(OMX_BUFFERHEADERTYPE*));
  assert(m_omxOutputBuffers != NULL);

  for (int i = 0; i < config.inBufferCount; i++) {
    omxError = OMX_AllocateBuffer(m_omxEncoder,
                                  &m_omxInputBuffers[i],
                                  (OMX_U32) EncoderConfig::IMG_COMP_PORT_INDEX_IN,
                                  NULL,
                                  config.inBufferSize);
    DEBUG_PRINT(ENCODER_PCOLOR "\nAllocate OMX inBuffer %d of size %d returned %x\n" KNRM,
                i, config.inBufferSize, omxError);
    assert(OMX_ErrorNone == omxError);
  }

  for (int i = 0; i < config.outBufferCount; i++) {
    omxError = OMX_AllocateBuffer(m_omxEncoder,
                                  &m_omxOutputBuffers[i],
                                  (OMX_U32) EncoderConfig::IMG_COMP_PORT_INDEX_OUT,
                                  NULL,
                                  config.outBufferSize);
    DEBUG_PRINT(ENCODER_PCOLOR "\nAllocate OMX outBuffer %d of size %d returned %x\n" KNRM,
                i, config.inBufferSize, omxError);
    assert(OMX_ErrorNone == omxError);
  }

  // TODO(mereweth)
  // wait for EventHandler with OMX_EventCmdComplete, OMX_StateIdle
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nWait for OMX_StateIdle at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  assert(0 == pthread_mutex_lock(&m_omxEncoderStateLock));
  while (m_omxEncoderState != OMX_StateIdle) {
    assert(0 == pthread_cond_wait(&m_omxEncoderStateChange, &m_omxEncoderStateLock));
  }
  assert(0 == pthread_mutex_unlock(&m_omxEncoderStateLock));
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
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nWait for OMX_StateIdle at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  assert(0 == pthread_mutex_lock(&m_omxEncoderStateLock));
  while (m_omxEncoderState != OMX_StateIdle) {
    assert(0 == pthread_cond_wait(&m_omxEncoderStateChange, &m_omxEncoderStateLock));
  }
  assert(0 == pthread_mutex_unlock(&m_omxEncoderStateLock));

  omxError = OMX_SendCommand(m_omxEncoder, OMX_CommandStateSet,
                             (OMX_U32)OMX_StateLoaded, NULL);
  assert(OMX_ErrorNone == omxError);

  // TODO(mereweth)
  // free all input and output buffers

  // TODO(mereweth)
  // wait for EventHandler with OMX_EventCmdComplete, OMX_StateLoaded
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nWait for OMX_StateLoaded at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
  assert(0 == pthread_mutex_lock(&m_omxEncoderStateLock));
  while (m_omxEncoderState != OMX_StateLoaded) {
    assert(0 == pthread_cond_wait(&m_omxEncoderStateChange, &m_omxEncoderStateLock));
  }
  assert(0 == pthread_mutex_unlock(&m_omxEncoderStateLock));

  omxError = OMX_FreeHandle(m_omxEncoder);
  assert(OMX_ErrorNone == omxError);

  omxError = OMX_Deinit();
  assert(OMX_ErrorNone == omxError);

  assert(0 == pthread_mutex_destroy(&m_omxEncoderStateLock));

  assert(0 == pthread_cond_destroy(&m_omxEncoderStateChange));
}

OMX_ERRORTYPE Encoder::eventCallback(OMX_IN OMX_HANDLETYPE compHandle,
                                     OMX_IN OMX_PTR context,
                                     OMX_IN OMX_EVENTTYPE event,
                                     OMX_IN OMX_U32 data1,
                                     OMX_IN OMX_U32 data2,
                                     OMX_IN OMX_PTR eventData)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\neventCallback at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  Encoder* comp = static_cast<Encoder*>(context);
  assert(comp);

  if (OMX_EventCmdComplete == event) {
    if (OMX_CommandStateSet == (OMX_COMMANDTYPE) data1) {
      assert(0 == pthread_mutex_lock(&comp->m_omxEncoderStateLock));

      comp->m_omxEncoderState = (OMX_STATETYPE) data2;

      assert(0 == pthread_cond_signal(&comp->m_omxEncoderStateChange));

      assert(0 == pthread_mutex_unlock(&comp->m_omxEncoderStateLock));
    }
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE Encoder::emptyDoneCallback(OMX_IN OMX_HANDLETYPE compHandle,
                                         OMX_IN OMX_PTR context,
                                         OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nemptyDoneCallback at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  Encoder* comp = static_cast<Encoder*>(context);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE Encoder::fillDoneCallback(OMX_IN OMX_HANDLETYPE compHandle,
                                        OMX_IN OMX_PTR context,
                                        OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nfillDoneCallback at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  Encoder* comp = static_cast<Encoder*>(context);
  return OMX_ErrorNone;
}
