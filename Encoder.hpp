#include "OMX_Core.h"
#include "OMX_Component.h"

#include "EncoderConfig.hpp"

#include <pthread.h>

#ifndef SDFLIGHT_CAMERAS_ENCODER_HPP
#define SDFLIGHT_CAMERAS_ENCODER_HPP

class Encoder
{
public:
  Encoder();

  ~Encoder();

private:
  static OMX_ERRORTYPE eventCallback(OMX_IN OMX_HANDLETYPE compHandle,
                                     OMX_IN OMX_PTR context,
                                     OMX_IN OMX_EVENTTYPE event,
                                     OMX_IN OMX_U32 data1,
                                     OMX_IN OMX_U32 data2,
                                     OMX_IN OMX_PTR eventData);

  static OMX_ERRORTYPE emptyDoneCallback(OMX_IN OMX_HANDLETYPE compHandle,
                                         OMX_IN OMX_PTR context,
                                         OMX_IN OMX_BUFFERHEADERTYPE* buffer);

  static OMX_ERRORTYPE fillDoneCallback(OMX_IN OMX_HANDLETYPE compHandle,
                                        OMX_IN OMX_PTR context,
                                        OMX_IN OMX_BUFFERHEADERTYPE* buffer);
  OMX_HANDLETYPE m_omxEncoder;

  OMX_STATETYPE m_omxEncoderState;

  OMX_BUFFERHEADERTYPE** m_omxInputBuffers;

  OMX_BUFFERHEADERTYPE** m_omxOutputBuffers;

  EncoderConfig::EncoderConfigType m_encoderConfig;

  pthread_mutex_t m_omxEncoderStateLock;

  pthread_cond_t m_omxEncoderStateChange;
}; // class Encoder

#endif // SDFLIGHT_CAMERAS_ENCODER_HPP
