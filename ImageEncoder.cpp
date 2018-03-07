#include "ImageEncoder.hpp"
#include "ImageEncoderConfig.hpp"
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include "Debug.hpp"

#include <media/hardware/HardwareAPI.h>

#include <qomx_core.h>

#ifdef __cplusplus
extern "C" {
#endif

OMX_ERRORTYPE OMX_Init_jpeg();

OMX_ERRORTYPE OMX_Deinit_jpeg();

OMX_ERRORTYPE OMX_GetHandle_jpeg(OMX_HANDLETYPE* handle,
                                 OMX_STRING componentName,
                                 OMX_PTR appData,
                                 OMX_CALLBACKTYPE* callBacks);

OMX_ERRORTYPE OMX_FreeHandle_jpeg(OMX_HANDLETYPE hComp);

#ifdef __cplusplus
}
#endif

#define ENCODER_PCOLOR KCYN

ImageEncoder::ImageEncoder() :
  m_omxEncoder(NULL),
  m_omxEncoderState(OMX_StateInvalid),
  m_omxInputBuffers(NULL),
  m_omxOutputBuffers(NULL),
  m_encoderConfig()
{
  OMX_ERRORTYPE omxError;
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nImageEncoder constructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  assert(0 == pthread_mutex_init(&m_omxEncoderStateLock, 0));

  assert(0 == pthread_cond_init(&m_omxEncoderStateChange, 0));

  omxError = OMX_Init_jpeg();
  assert(OMX_ErrorNone == omxError);

  OMX_CALLBACKTYPE callbacks = {
    .EventHandler = eventCallback,
    .EmptyBufferDone = emptyDoneCallback,
    .FillBufferDone = fillDoneCallback,
  };

  omxError = OMX_GetHandle_jpeg(&m_omxEncoder,
                                (OMX_STRING)"OMX.qcom.image.jpeg.encoder",
                                this, &callbacks);
  if (OMX_ErrorNone != omxError) {
    DEBUG_PRINT("Failed to find image.jpeg.encoder: 0x%x", omxError);
    assert(0);
  }

  ImageEncoderConfig::ImageEncoderConfigType config = {
      .codec = OMX_IMAGE_CodingJPEG, // OMX_IMAGE_CodingJPEG2K?
      .inBufferCount = 0,
      .outBufferCount = 0,
      .inBufferSize = 0,
      .outBufferSize = 0
  };

  omxError = ImageEncoderConfig::Configure(m_omxEncoder, config);
  assert(OMX_ErrorNone == omxError);
  m_encoderConfig = config;

  omxError = OMX_GetState(m_omxEncoder, &m_omxEncoderState);
  assert(OMX_ErrorNone == omxError);
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nOMX state %d at time %f\n" KNRM,
              m_omxEncoderState,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  // Transition to OMX_StateIdle
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
                                  (OMX_U32) ImageEncoderConfig::IMG_COMP_PORT_INDEX_IN,
                                  NULL,
                                  config.inBufferSize);
    DEBUG_PRINT(ENCODER_PCOLOR "\nAllocate OMX inBuffer %d of size %d returned %x\n" KNRM,
                i, config.inBufferSize, omxError);
    assert(OMX_ErrorNone == omxError);
  }

  for (int i = 0; i < config.outBufferCount; i++) {
    omxError = OMX_AllocateBuffer(m_omxEncoder,
                                  &m_omxOutputBuffers[i],
                                  (OMX_U32) ImageEncoderConfig::IMG_COMP_PORT_INDEX_OUT,
                                  NULL,
                                  config.outBufferSize);
    DEBUG_PRINT(ENCODER_PCOLOR "\nAllocate OMX outBuffer %d of size %d returned %x\n" KNRM,
                i, config.inBufferSize, omxError);
    assert(OMX_ErrorNone == omxError);
  }

  // wait for EventHandler with OMX_EventCmdComplete, OMX_StateIdle
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nWait for OMX_StateIdle at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  assert(0 == pthread_mutex_lock(&m_omxEncoderStateLock));
  while (m_omxEncoderState != OMX_StateIdle) {
    assert(0 == pthread_cond_wait(&m_omxEncoderStateChange, &m_omxEncoderStateLock));
  }
  assert(0 == pthread_mutex_unlock(&m_omxEncoderStateLock));

  // Transition to OMX_StateExecuting
  omxError = OMX_SendCommand(m_omxEncoder, OMX_CommandStateSet,
                             (OMX_U32)OMX_StateExecuting, NULL);
  assert(OMX_ErrorNone == omxError);

  // wait for EventHandler with OMX_EventCmdComplete, OMX_StateExecuting
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nWait for OMX_StateExecuting at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  assert(0 == pthread_mutex_lock(&m_omxEncoderStateLock));
  while (m_omxEncoderState != OMX_StateExecuting) {
    assert(0 == pthread_cond_wait(&m_omxEncoderStateChange, &m_omxEncoderStateLock));
  }
  assert(0 == pthread_mutex_unlock(&m_omxEncoderStateLock));
}

ImageEncoder::~ImageEncoder()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nImageEncoder destructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  OMX_ERRORTYPE omxError;

  omxError = OMX_SendCommand(m_omxEncoder, OMX_CommandStateSet,
                             (OMX_U32)OMX_StateIdle, NULL);
  assert(OMX_ErrorNone == omxError);

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

  // free all input and output buffers
  for (int i = 0; i < m_encoderConfig.inBufferCount; i++) {
    omxError = OMX_FreeBuffer(m_omxEncoder,
                              (OMX_U32) ImageEncoderConfig::IMG_COMP_PORT_INDEX_IN,
                              m_omxInputBuffers[i]);
    m_omxInputBuffers[i] = NULL;
    DEBUG_PRINT(ENCODER_PCOLOR "\nFree OMX inBuffer %d returned %x\n" KNRM,
                i, omxError);
    assert(OMX_ErrorNone == omxError);
  }

  for (int i = 0; i < m_encoderConfig.outBufferCount; i++) {
    omxError = OMX_FreeBuffer(m_omxEncoder,
                              (OMX_U32) ImageEncoderConfig::IMG_COMP_PORT_INDEX_OUT,
                              m_omxOutputBuffers[i]);
    m_omxOutputBuffers[i] = NULL;
    DEBUG_PRINT(ENCODER_PCOLOR "\nFree OMX outBuffer %d returned %x\n" KNRM,
                i, omxError);
    assert(OMX_ErrorNone == omxError);
  }

  assert(m_omxInputBuffers != NULL);
  free(m_omxInputBuffers);
  m_omxInputBuffers = NULL;

  assert(m_omxOutputBuffers != NULL);
  free(m_omxOutputBuffers);
  m_omxOutputBuffers = NULL;

  // wait for EventHandler with OMX_EventCmdComplete, OMX_StateLoaded
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nWait for OMX_StateLoaded at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);
  assert(0 == pthread_mutex_lock(&m_omxEncoderStateLock));
  while (m_omxEncoderState != OMX_StateLoaded) {
    assert(0 == pthread_cond_wait(&m_omxEncoderStateChange, &m_omxEncoderStateLock));
  }
  assert(0 == pthread_mutex_unlock(&m_omxEncoderStateLock));

  omxError = OMX_FreeHandle_jpeg(m_omxEncoder);
  assert(OMX_ErrorNone == omxError);

  omxError = OMX_Deinit_jpeg();
  assert(OMX_ErrorNone == omxError);

  assert(0 == pthread_mutex_destroy(&m_omxEncoderStateLock));

  assert(0 == pthread_cond_destroy(&m_omxEncoderStateChange));
}

OMX_ERRORTYPE ImageEncoder::eventCallback(OMX_IN OMX_HANDLETYPE compHandle,
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

  ImageEncoder* comp = static_cast<ImageEncoder*>(context);
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

OMX_ERRORTYPE ImageEncoder::emptyDoneCallback(OMX_IN OMX_HANDLETYPE compHandle,
                                         OMX_IN OMX_PTR context,
                                         OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nemptyDoneCallback at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  ImageEncoder* comp = static_cast<ImageEncoder*>(context);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ImageEncoder::fillDoneCallback(OMX_IN OMX_HANDLETYPE compHandle,
                                        OMX_IN OMX_PTR context,
                                        OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nfillDoneCallback at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  ImageEncoder* comp = static_cast<ImageEncoder*>(context);
  return OMX_ErrorNone;
}
