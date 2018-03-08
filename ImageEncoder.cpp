#include "ImageEncoder.hpp"
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include "Debug.hpp"
#include <stdlib.h>

//#include <media/hardware/HardwareAPI.h>

//#include <qomx_core.h>

#define ENCODER_PCOLOR KCYN

ImageEncoder::ImageEncoder()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  DEBUG_PRINT(ENCODER_PCOLOR "\nImageEncoder constructor at time %f\n" KNRM,
              tv.tv_sec + tv.tv_usec / 1000000.0);

  for (int i = 0; i < IMGENC_IMAGE_MODE_MAX; i++) {
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
    m_inputs[i].input.addr = (uint8_t *)malloc(m_inputs[i].input.size);
    assert(m_inputs[i].input.addr);

    m_inputs[i].output.size = m_inputs[i].dim.w * m_inputs[i].dim.h * 3 / 2;
    m_inputs[i].output.addr = (uint8_t *)malloc(m_inputs[i].output.size);
    assert(m_inputs[i].output.addr);

    /* set encode parameters */
    m_inputs[i].params.jpeg_cb = NULL; //mm_jpeg_encode_callback;
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

    m_inputs[i].params.encode_thumbnail = 1;
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
    free(m_inputs[i].input.addr);
    m_inputs[i].input.addr = NULL;

    assert(m_inputs[i].output.addr);
    free(m_inputs[i].output.addr);
    m_inputs[i].output.addr = NULL;
  }
}
