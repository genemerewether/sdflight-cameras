#include "ImageEncoderConfig.hpp"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "mm_jpeg_interface.h"
#include "mm_jpeg_ionbuf.h"
#ifdef __cplusplus
}
#endif

#ifndef SDFLIGHT_CAMERAS_IMAGEENCODER_HPP
#define SDFLIGHT_CAMERAS_IMAGEENCODER_HPP

class ImageEncoder
{
public:
  typedef enum {
    IMGENC_IMG_13MP,
    IMGENC_IMG_2MP,
    IMGENC_IMG_VGA,
    IMGENC_IMAGE_MODE_MAX
  } ImageEncoderInputType;

  ImageEncoder();

  ~ImageEncoder();

  buffer_t* getPictureInBuffer(ImageEncoderInputType type);

  int startJob(ImageEncoderInputType type);

  int writeBuffer(ImageEncoderInputType type,
                  const char* const fileName);

  int waitJob(unsigned int seconds);

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

  uint8_t** m_inputBuffers;
  uint8_t** m_outputBuffers;

  ImageEncoderConfig::ImageEncoderConfigType m_encoderConfig;

  pthread_mutex_t m_omxEncoderStateLock;

  pthread_cond_t m_omxEncoderStateChange;

  bool m_frameReady;

  pthread_mutex_t m_encoderFrameLock;

  pthread_cond_t m_encoderFrameReady;

}; // class ImageEncoder

#endif // SDFLIGHT_CAMERAS_IMAGEENCODER_HPP
