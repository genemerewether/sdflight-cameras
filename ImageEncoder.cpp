#include "ImageEncoder.hpp"
#include "Debug.hpp"

#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

//#include <media/hardware/HardwareAPI.h>

//#include <qomx_core.h>

#define ENCODER_PCOLOR KCYN

ImageEncoder::ImageEncoder() :
  m_inputs(),
  m_jobRunning(false),
  m_frameReady(false)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nImageEncoder constructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  assert(0 == pthread_mutex_init(&m_encoderFrameLock, 0));

  assert(0 == pthread_cond_init(&m_encoderFrameReady, 0));

  for (int i = 0; i < IMGENC_IMAGE_MODE_MAX; i++) {
    m_inputs[i].imageEncPtr = this;

    switch (i) {
      case IMGENC_IMG_13MP:
        m_inputs[i].dim.w = 4208;
        m_inputs[i].dim.h = 3120;
        break;
      case IMGENC_IMG_2MP:
        m_inputs[i].dim.w = 1920;
        m_inputs[i].dim.h = 1080;
        break;
      case IMGENC_IMG_VGA:
        m_inputs[i].dim.w = 640;
        m_inputs[i].dim.h = 480;
        break;
      default:
        DEBUG_PRINT(KRED "\nImageEncoder setup found invalid mode %d\n" KNRM, i);
        assert(0);
    }

    // malloc the buffers
    // NOTE(mereweth) - can use ION buffers for input
    m_inputs[i].input.size = m_inputs[i].dim.w * m_inputs[i].dim.h * 3 / 2;
    //m_inputs[i].input.addr = (uint8_t *)malloc(m_inputs[i].input.size);
    m_inputs[i].input.addr = (uint8_t*) buffer_allocate(&(m_inputs[i].input), 0); // not cached
    assert(m_inputs[i].input.addr);

    m_inputs[i].output.size = m_inputs[i].dim.w * m_inputs[i].dim.h * 3 / 2;
    //m_inputs[i].output.addr = (uint8_t *)malloc(m_inputs[i].output.size);
    m_inputs[i].output.addr = (uint8_t*) buffer_allocate(&(m_inputs[i].output), 0); // not cached
    assert(m_inputs[i].output.addr);

    /* set encode parameters */
    m_inputs[i].params.jpeg_cb = imageEncCallback;
    m_inputs[i].params.userdata = &m_inputs[i];
    m_inputs[i].params.color_format = MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;
    m_inputs[i].params.thumb_color_format = MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;

    /* dest buffer config */
    m_inputs[i].params.dest_buf[0].buf_size = m_inputs[i].output.size;
    m_inputs[i].params.dest_buf[0].buf_vaddr = m_inputs[i].output.addr;
    m_inputs[i].params.dest_buf[0].fd = m_inputs[i].output.p_pmem_fd;
    m_inputs[i].params.dest_buf[0].index = 0;
    m_inputs[i].params.num_dst_bufs = 1;

    /* src buffer config*/
    m_inputs[i].params.src_main_buf[0].buf_size = m_inputs[i].input.size;
    m_inputs[i].params.src_main_buf[0].buf_vaddr = m_inputs[i].input.addr;
    m_inputs[i].params.src_main_buf[0].fd = m_inputs[i].input.p_pmem_fd;
    m_inputs[i].params.src_main_buf[0].index = 0;
    m_inputs[i].params.src_main_buf[0].format = MM_JPEG_FMT_YUV;
    int size = m_inputs[i].dim.w * m_inputs[i].dim.h;
    m_inputs[i].params.src_main_buf[0].offset.mp[0].len = size;
    m_inputs[i].params.src_main_buf[0].offset.mp[1].len = size >> 1;
    m_inputs[i].params.num_src_bufs = 1;

    m_inputs[i].params.encode_thumbnail = 0;
    m_inputs[i].params.quality = 80;

    m_inputs[i].job.encode_job.dst_index = 0;
    m_inputs[i].job.encode_job.src_index = 0;
    m_inputs[i].job.encode_job.rotation = 0;

    /* main dimension */
    m_inputs[i].job.encode_job.main_dim.src_dim.width = m_inputs[i].dim.w;
    m_inputs[i].job.encode_job.main_dim.src_dim.height = m_inputs[i].dim.h;
    m_inputs[i].job.encode_job.main_dim.dst_dim.width = m_inputs[i].dim.w;
    m_inputs[i].job.encode_job.main_dim.dst_dim.height = m_inputs[i].dim.h;
    m_inputs[i].job.encode_job.main_dim.crop.top = 0;
    m_inputs[i].job.encode_job.main_dim.crop.left = 0;
    m_inputs[i].job.encode_job.main_dim.crop.width = m_inputs[i].dim.w;
    m_inputs[i].job.encode_job.main_dim.crop.height = m_inputs[i].dim.h;

    /* thumb dimension */
    m_inputs[i].job.encode_job.thumb_dim.src_dim.width = m_inputs[i].dim.w;
    m_inputs[i].job.encode_job.thumb_dim.src_dim.height = m_inputs[i].dim.h;
    m_inputs[i].job.encode_job.thumb_dim.dst_dim.width = 320;
    m_inputs[i].job.encode_job.thumb_dim.dst_dim.height = 240;
    m_inputs[i].job.encode_job.thumb_dim.crop.top = 0;
    m_inputs[i].job.encode_job.thumb_dim.crop.left = 0;
    m_inputs[i].job.encode_job.thumb_dim.crop.width = m_inputs[i].dim.w;
    m_inputs[i].job.encode_job.thumb_dim.crop.height = m_inputs[i].dim.h;

    m_inputs[i].job.encode_job.exif_info.numOfEntries = 0;

    // Open jpeg sessions
    m_inputs[i].handle = jpeg_open(&m_inputs[i].ops, m_inputs[i].dim);
    assert(m_inputs[i].handle);
    assert(m_inputs[i].ops.create_session);

    gettimeofday(&tv, NULL);
    DEBUG_PRINT(ENCODER_PCOLOR "\nImageEncoder creating session %d at time %f\n" KNRM,
                i, tv.tv_sec + tv.tv_usec / 1000000.0);

    m_inputs[i].ops.create_session(m_inputs[i].handle, &m_inputs[i].params,
                                   &m_inputs[i].job.encode_job.session_id);
    assert(m_inputs[i].job.encode_job.session_id);

    m_inputs[i].job.job_type = JPEG_JOB_TYPE_ENCODE;
  }
}

ImageEncoder::~ImageEncoder()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nImageEncoder destructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  for (int i = 0; i < IMGENC_IMAGE_MODE_MAX; i++) {
    assert(m_inputs[i].input.addr);
    assert(0 == buffer_deallocate(&(m_inputs[i].input)));
    //free(m_inputs[i].input.addr);
    m_inputs[i].input.addr = NULL;

    assert(m_inputs[i].output.addr);
    assert(0 == buffer_deallocate(&(m_inputs[i].output)));
    //free(m_inputs[i].output.addr);
    m_inputs[i].output.addr = NULL;

    m_inputs[i].ops.destroy_session(m_inputs[i].job.encode_job.session_id);
    m_inputs[i].ops.close(m_inputs[i].handle);
  }
}

buffer_t* ImageEncoder::getPictureInBuffer(ImageEncoderInputType type)
{
  return &m_inputs[type].input;
}

int ImageEncoder::startJob(ImageEncoderInputType type)
{
  if (m_jobRunning) {
    return -1;
  }
  /* NOTE(mereweth) - could make this an array to allow simultaneous jobs
   * of different types
   */
  m_frameReady = false;
  // TODO(mereweth) - how many job IDs?
  int stat = m_inputs[type].ops.start_job(&m_inputs[type].job,
                                          &m_inputs[type].jobID);
  if (!stat) {
    m_jobRunning = true;
  }

  return stat;
}

int ImageEncoder::writeBuffer(ImageEncoderInputType type,
                              const char* const fileName)
{
  int fid = open(fileName,
                 O_CREAT | O_WRONLY | O_TRUNC,
                 S_IRUSR | S_IWUSR |  S_IRGRP | S_IWGRP);
  if (fid == -1) {
    DEBUG_PRINT(KRED "ImageEncoder failed to open %s\n" KNRM, fileName);
    return errno;
  }

  int stat = write(fid,
                   m_inputs[type].output.addr,
                   m_inputs[type].output.size);
  if (stat == -1) {
    DEBUG_PRINT(KRED "ImageEncoder failed to write %s, fid %d, bytes %d\n" KNRM,
                fileName, fid, m_inputs[type].output.size);
    (void) close(fid);
    return errno;
  }
  DEBUG_PRINT(ENCODER_PCOLOR "ImageEncoder wrote %s, fid %d, bytes %d\n" KNRM,
              fileName, fid, m_inputs[type].output.size);

  stat = close(fid);
  if (stat == -1) {
    DEBUG_PRINT(KRED "ImageEncoder failed to close %s, fid %d\n" KNRM,
                fileName, fid);
    return errno;
  }

  return 0;
}

int ImageEncoder::waitJob(unsigned int seconds)
{
  assert(0 == pthread_mutex_lock(&m_encoderFrameLock));

  struct timeval now;
  gettimeofday(&now,NULL);
  struct timespec wait;
  wait.tv_sec = seconds + now.tv_sec;
  wait.tv_nsec = now.tv_usec * 1000;

  int stat = 0;
  while (!m_frameReady && stat == 0) {
    stat = pthread_cond_timedwait(&m_encoderFrameReady, &m_encoderFrameLock,
				  &wait);
  }
  if (stat == ETIMEDOUT) {
    DEBUG_PRINT(KRED "ImageEncoder timed out in waitJob\n" KNRM);
  }

  assert(0 == pthread_mutex_unlock(&m_encoderFrameLock));

  return stat;
}

void ImageEncoder::innerCallback(ImageEncoderInputType type,
                                 mm_jpeg_output_t* jpgOut)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  if (jpgOut) {
    DEBUG_PRINT(ENCODER_PCOLOR "\nImageEncoder innerCallback type %d, size %d at time %f\n" KNRM,
                (int) type, jpgOut->buf_filled_len,
                tv.tv_sec + tv.tv_usec / 1000000.0);
  }
  else {
    DEBUG_PRINT(KRED "\nImageEncoder empty innerCallback type %d at time %f\n" KNRM,
                (int) type, tv.tv_sec + tv.tv_usec / 1000000.0);
  }

  m_jobRunning = false;
  
  assert(0 == pthread_mutex_lock(&m_encoderFrameLock));

  m_frameReady = true;

  assert(0 == pthread_cond_signal(&m_encoderFrameReady));

  assert(0 == pthread_mutex_unlock(&m_encoderFrameLock));
}

void ImageEncoder::imageEncCallback(jpeg_job_status_t status,
                                    uint32_t client_hdl,
                                    uint32_t jobId,
                                    mm_jpeg_output_t *p_output,
                                    void *userData)
{
  assert(userData);
  ImageEncoderInterface* imageEncIntf = (ImageEncoderInterface*) userData;
  ImageEncoder* comp = (ImageEncoder*) imageEncIntf->imageEncPtr;
  assert(comp);
  // TODO(mereweth) - assert the function pointer
  comp->innerCallback(imageEncIntf->imageEncInputType,
                      p_output);
}
