package com.hellw.ffmpegh264;

/**
 * H264编码器
 * Created by hellw on 2018/9/18.
 */

public class H264Encoder {

    static {
        System.loadLibrary("x264");
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");

        System.loadLibrary("video_play");
    }

    /**
     * 初始化H264编码
     * @param destUrl 推流url
     * @param w 视频宽
     * @param h 视频高
     * @return 1 成功
     */
    public native int init(String destUrl, int w, int h);

    /**
     * 编码并推送
     * @param data nv21格式数据
     * @param w 视频宽
     * @param h 视频高
     * @return 1 成功
     */
    public native int encoder(byte[] data, int w, int h);

    /**
     * 回收资源并释放H264 写h264尾
     * @return 1 成功
     */
    public native int destroy();


}
