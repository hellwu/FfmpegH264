#include "com_hellw_ffmpegh264_MainActivity.h"
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

AVCodecParserContext *m_pCodecParserCtx;
AVCodecContext *m_pCodecCtx;
AVCodec *m_codec;
AVPacket m_avpkt;
AVFrame *m_pFrameYUV;
AVFrame *m_picture;
struct SwsContext *m_pImgCtx;
int m_PicBytes;
uint8_t *m_PicBuf;

 JNIEXPORT int JNICALL Java_com_hellw_ffmpegh264_MainActivity_init
   (JNIEnv * env, jobject jobj) {
 av_init_packet(&m_avpkt);

    m_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if(!m_codec){
        LOGI("Codec not found\n");
        return -1;
    }
    m_pCodecCtx = avcodec_alloc_context3(m_codec);
    if(!m_pCodecCtx){
        LOGI("Could not allocate video codec context\n");
        return -1;
    }

    m_pCodecParserCtx=av_parser_init(AV_CODEC_ID_H264);
    if (!m_pCodecParserCtx){
        LOGI("Could not allocate video parser context\n");
        return -1;
    }

    if(m_codec->capabilities&CODEC_CAP_TRUNCATED)
        m_pCodecCtx->flags|= CODEC_FLAG_TRUNCATED;

    if (avcodec_open2(m_pCodecCtx, m_codec, NULL) < 0) {
        LOGI("Could not open codec\n");
        return -1;
    }

    m_picture = av_frame_alloc();
    m_pFrameYUV = av_frame_alloc();
    if(!m_picture || !m_pFrameYUV){
        LOGI("Could not allocate video frame\n");
        return -1;
    }

    m_PicBytes = 0;
    m_PicBuf = NULL;
    m_pImgCtx = NULL;
}