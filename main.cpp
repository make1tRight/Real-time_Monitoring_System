#include <iostream>
#include <fstream>
#include "media_render.h"
#include "media_codec.h"
#include "media_encoder.h"
#include "media_util.h"
#include "media_decoder.h"


int main(int argc, char* argv[])
{
    //打开媒体文件
    const char* url = "test2.mp4";
    //解封装输入上下文
    AVFormatContext* ic = nullptr;
    auto ret = avformat_open_input(&ic, url,
        NULL,		//封装器格式, NULL表示根据后缀名自动检测
        NULL		//rtsp需要设置
    );
    if (ret != 0) {
        PrintError(ret);
        return -1;
    }
    //获取媒体信息(无头部格式)
    ret = avformat_find_stream_info(ic, NULL);
    if (ret < 0) {
        PrintError(ret);
        return -1;
    }
    av_dump_format(ic, 0, url,
        0//is_output=0表示输入
    );
    AVStream* as = nullptr;//音频流
    AVStream* vs = nullptr;//视频流
    for (int i = 0; i < ic->nb_streams; ++i) {
        //如果是音频
        if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            as = ic->streams[i];
            std::cout << "========Audio========" << std::endl;
            std::cout << "sample_rate=" << as->codecpar->sample_rate << std::endl;
            std::cout << "========Audio========" << std::endl;
        }
        //如果是视频
        if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            vs = ic->streams[i];
            std::cout << "========Video========" << std::endl;
            std::cout << "width=" << vs->codecpar->width << std::endl;
            std::cout << "height=" << vs->codecpar->height << std::endl;
            std::cout << "========Video========" << std::endl;
        }
    }



    auto video_codec_id = vs->codecpar->codec_id;	//视频编码器ID
    auto video_par = vs->codecpar;					//视频编码参数
    /* 视频解码器初始化 */
    MediaDecoder media_decoder;
    auto decode_c = MediaCodec::CreateContext(video_codec_id, MediaCodec::DECODER);
    //解封装的视频编码参数传递给解码类
    avcodec_parameters_to_context(decode_c, video_par);
    //设置到解码器中, 保证线程安全, 设置完后decode_c不能在类外部使用
    media_decoder.SetContext(decode_c);
    if (media_decoder.OpenContext() == false) {
        std::cout << "decode Open failed..." << std::endl;
        return -1;
    }
    /* 创建解码输出的空间 */
    auto frame = media_decoder.CreateAVFrame();
    /* 渲染的初始化 */
    auto view = MediaRender::CreateContext();
    view->Init(video_par->width, video_par->height, (AVPixelFormat)video_par->format);

    AVPacket pkt;
    for (;;) {
        ret = av_read_frame(ic, &pkt);
        if (ret != 0) {
            PrintError(ret);
            return -1;
        }
        if (vs && pkt.stream_index == vs->index) {
            std::cout << "Video: ";
            //解码视频
            if (media_decoder.SendPacket(&pkt)) {
                while (media_decoder.RecvFrame(frame)) {
                    std::cout << frame->pts << " " << std::endl;
                    view->PresentFrame(frame);//渲染视频
                }
            }
        }
        if (as && pkt.stream_index == as->index) {
            std::cout << "Audio: ";
        }
        std::cout << pkt.pts << ": " << pkt.dts << ": " << pkt.size << std::endl;
        av_packet_unref(&pkt);//保证了内存可以及时释放
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    av_frame_free(&frame);
    avformat_close_input(&ic);//与avformat_open_input成对使用
    return 0;
}