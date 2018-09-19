#include "com_hellw_ffmpegh264_H264Encoder.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

AVFormatContext *ofCtx;
AVCodecContext *m_pCodecCtx;
AVStream *m_video_st;
AVCodec *m_codec;
AVPacket m_avpkt;
AVFrame *m_pFrameYUV;
int result;
uint8_t *m_PicBuf;
int size;
long count;

static int init(const char *dest_url, int w, int h);

static int encoder(uint8_t *data, int w, int h);

static int destroy();

int result;

JNIEXPORT jint JNICALL Java_com_hellw_ffmpegh264_H264Encoder_init
        (JNIEnv *env, jobject jobj, jstring jdestUrl, jint jw, jint jh) {
    const char *desturl = (*env)->GetStringUTFChars(env, jdestUrl, 0);

    result = init(desturl, jw, jh);
    count = 0;
    (*env)->ReleaseStringUTFChars(env, jdestUrl, desturl);
    return result;
}

/*
 * Class:     com_hellw_ffmpegh264_H264Encoder
 * Method:    encoder
 * Signature: ([BII)I
 */
JNIEXPORT jint JNICALL Java_com_hellw_ffmpegh264_H264Encoder_encoder
        (JNIEnv *env, jobject jobj, jbyteArray jdatas, jint jw, jint jh) {
    jbyte *bytes = (*env)->GetByteArrayElements(env, jdatas, NULL);

    result = encoder((uint8_t *) bytes, jw, jh);
    (*env)->ReleaseByteArrayElements(env, jdatas, bytes, 0);
    return result;
}

/*
 * Class:     com_hellw_ffmpegh264_H264Encoder
 * Method:    destroy
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_hellw_ffmpegh264_H264Encoder_destroy
        (JNIEnv *env, jobject jobj) {
    return destroy();
}

static int init(const char *dest_url, int w, int h) {
    AVOutputFormat *fmt;
    int ret;

    //Register all the codecs
    avcodec_register_all();
    LOGI("register all\n");
    LOGI("init() dest_url = %s\n", dest_url);
    avformat_alloc_output_context2(&ofCtx, NULL, "flv", dest_url);

    fmt = ofCtx->oformat;
    if ((ret = avio_open((*ofCtx).pb, dest_url, AVIO_FLAG_WRITE)) < 0) {
        LOGI("avio_open failure ret = %d", ret);
        return -1;
    }

    m_video_st = avformat_new_stream(fmt, NULL);
    if (m_video_st == NULL) {
        LOGI("new stream failure\n");
        return -1;
    }

    m_pCodecCtx = m_video_st->codec;

    m_pCodecCtx->codec_id = AV_CODEC_ID_H264;
    m_pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    m_pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_pCodecCtx->width = w;
    m_pCodecCtx->height = h;
    //����
    m_pCodecCtx->bit_rate = 400000;
    //gop��С
    m_pCodecCtx->gop_size = 200;

    // ֡�ʵĻ�����λ�÷�����ʾ
    m_pCodecCtx->time_base.den = 1;
    m_pCodecCtx->time_base.num = FPS;
    //û��b֡
    m_pCodecCtx->max_b_frames = 0;
    //H264Ĭ��: 51(ע:�°�x264������Ϊ69)
    //��qpmin�෴�����趨��x264���õ��������ֵ��Ĭ��ֵ51��H.264�淶����ߵĿ���ֵ����������������Ĭ��ֵ�����ǽ�����qpmax
    m_pCodecCtx->qmax = 51;
    //Ĭ��: 0
    //�趨x264��ʹ�õ���С����ֵ������ֵԽ�ͣ������ƵԽ�ӽ�������Ƶ���͵�һ���̶�ʱ�����������ȥ��������ͬ����Ȼ��������ȫ��ͬ��ͨ��û����������x264�ٻ��ѱ����������ʱ������ˡ�
    m_pCodecCtx->qmin = 10;

    AVDictionary *param = 0;
    if (m_pCodecCtx->codec_id == AV_CODEC_ID_H264) {
        //ѹ���ʺͱ����ٶȼ��ƽ��
        av_dict_set(&param, "preset", "superfast", 0);
        //uneѡ�����������Ƶ�����ݽ����Ż���ָ��tuning���������ĸı����� �Cpreset�ĸı䣬��������������
        av_dict_set(&param, "tune", "zerolatency", 0);
        LOGI("set h264 param finished!\n");
    }

    m_codec = avcodec_find_encoder(m_pCodecCtx->codec_id);
    if (m_codec == NULL) {
        LOGI("find encoder failure\n");
        return -1;
    }

    if ((ret = avcodec_open2(m_pCodecCtx, m_codec, &param)) < 0) {
        LOGI("avcodec_open2 failure\n");
        return -1;
    }
    //write file header
    avformat_write_header(ofCtx, NULL);
    return 0;
}


static int encoder(unsigned char *data, int w, int h) {
    m_pFrameYUV = av_frame_alloc();
    int picture_size = av_image_get_buffer_size(m_pCodecCtx->pix_fmt, m_pCodecCtx->width, m_pCodecCtx->height, 1);
    m_PicBuf = av_malloc(picture_size);
    int j, ret;
    av_image_fill_arrays(m_pFrameYUV->data, m_pFrameYUV->linesize, m_PicBuf, m_pCodecCtx->pix_fmt, m_pCodecCtx->width, m_pCodecCtx->height, 1);

    //H264 1��AVPacket��Ӧһ��NAL
    av_init_packet(&m_avpkt);
    size = m_pCodecCtx->width * m_pCodecCtx->height;
    //NV21תYUV420p
    //Y����
    memcpy(m_pFrameYUV, data, size);
    for (j = 0; j < size / 4; j++) {
        *(m_pFrameYUV->data[2] + j) = *(data + size + j * 2); // V
        *(m_pFrameYUV->data[1] + j) = *(data + size + j * 2 + 1); //U
    }

    m_pFrameYUV->format = AV_PIX_FMT_YUV420P;
    m_pFrameYUV->width = w;
    m_pFrameYUV->height = h;

    ret = avcodec_send_frame(m_pCodecCtx, m_pFrameYUV);
    if(ret != 0) {
        LOGI("avcodec_send_frame failure!\n");
        return -1;
    }

    ret = avcodec_receive_packet(m_pCodecCtx, &m_avpkt);
    av_frame_free(&m_pFrameYUV);
    if(ret != 0 || m_avpkt.size <= 0) {
        LOGI("avcodec_receive_packet failure!\n");
        return -1;
    }

    //��������־
    m_avpkt.stream_index = m_video_st->index;

    AVRational time_base = m_video_st->time_base;
    m_avpkt.pts = count * time_base.den / (time_base.num * FPS);
    //û��B֡ pts dts��ͬ
    m_avpkt.dts = m_avpkt.pts;
    m_avpkt.duration = time_base.den / (time_base.num * FPS);
    m_avpkt.pos = -1;

    LOGI("count = %ld, pts = %lld, duration = %lld, time_base(%lld, %lld)\n", count, m_avpkt.pts, m_avpkt.duration, time_base.den, time_base.num);
    ret = av_interleaved_write_frame(ofCtx, &m_avpkt);
    if(ret != 0) {
        LOGI("av_interleaved_write_frame failure, count = %ld\n", count);
        return -1;
    }
    count++;
    return 0;
}

static int destroy() {
    av_free(m_PicBuf);
    if(m_video_st) {
        avcodec_close(m_pCodecCtx);
    }

    if(ofCtx) {
        avio_close(ofCtx->pb);
        avformat_free_context(ofCtx);
        ofCtx = NULL;
    }
    return 0;
}