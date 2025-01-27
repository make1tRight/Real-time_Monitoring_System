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

    const char* out_url = "out_250126.mp4";
    AVFormatContext* ec = nullptr;
    ret = avformat_alloc_output_context2(&ec, NULL, NULL,
        out_url);//根据文件名匹配封装格式
    if (ret < 0) {
        PrintError(ret);
        return -1;
    }
    //添加视频流与音频流, mvs, mas会随着context的销毁而销毁
    auto mvs = avformat_new_stream(ec, NULL);//视频流
    auto mas = avformat_new_stream(ec, NULL);//音频流
    ret = avio_open(&ec->pb, out_url, AVIO_FLAG_WRITE);
    if (ret < 0) {
        PrintError(ret);
        return -1;
    }
    if (vs) {
        mvs->time_base = vs->time_base;//与原视频的time_base一致
        //从解封装中复制参数
        avcodec_parameters_copy(mvs->codecpar, vs->codecpar);
    }
    if (as) {
        mas->time_base = as->time_base;
        avcodec_parameters_copy(mas->codecpar, as->codecpar);
    }
    //写入文件头
    ret = avformat_write_header(ec, NULL);
    if (ret < 0) {
        PrintError(ret);
        return -1;
    }
    //打印输出上下文
    av_dump_format(ec, 0, out_url, 1);

    double begin_sec = 10.0;//截取开始时间
    double end_sec = begin_sec + 20.0;	//截取结束时间
    long long begin_pts = 0;//视频开始时间
    long long begin_audio_pts = 0;//音频开始时间
    long long end_pts = 0;

    //换算成pts, 换算成输入ic的pts, 以视频流为准
    if (vs && vs->time_base.num > 0) {
        double t = (double)vs->time_base.den / (double)vs->time_base.num;
        begin_pts = (long long)(begin_sec * t);
        end_pts = (long long)(end_sec * t);
    }
    if (as && as->time_base.num > 0) {
        double t = (double)as->time_base.den / (double)as->time_base.num;
        begin_audio_pts = (long long)(begin_sec * t);
    }
    //AVSEEK_FLAG_ANY
    if (vs) {
        ret = av_seek_frame(ic, vs->index,
            begin_pts, AVSEEK_FLAG_ANY|AVSEEK_FLAG_BACKWARD);//向后或关键帧
        if (ret < 0) {
            PrintError(ret);
            return -1;
        }
    }
    AVPacket pkt;
    for (;;) {
        ret = av_read_frame(ic, &pkt);
        if (ret != 0) {
            PrintError(ret);
            return -1;
        }
        AVStream* in_stream = ic->streams[pkt.stream_index];
        AVStream* out_stream = nullptr;
        long long offset_pts = 0;//偏移pts, 用于截断的开头pts运算
        if (vs && pkt.stream_index == vs->index) {
            std::cout << "Video: ";
            //解码视频
            if (pkt.pts < begin_pts - 10) {
                av_packet_unref(&pkt);
                continue;
            }
            if (pkt.pts > end_pts) {
                av_packet_unref(&pkt);//保证了内存可以及时释放
                break;
            }
            out_stream = ec->streams[0];
            offset_pts = begin_pts;
        }
        if (as && pkt.stream_index == as->index) {
            std::cout << "Audio: ";
            // 如果音频帧的时间戳超过视频的结束时间，丢弃该音频帧
            if (pkt.pts < begin_pts - 10) {
                av_packet_unref(&pkt);
                continue;
            }
            out_stream = ec->streams[1];
            offset_pts = begin_audio_pts;
        }
        std::cout << pkt.pts << ": " << pkt.dts << ": " << pkt.size << std::endl;
        //重新计算pts, dts, duration
        //写入音视频帧, 会清理pkt
        //The operation is mathematically equivalent to `a* bq(输入timebase) / cq(输出timebase)`.
        pkt.pts = av_rescale_q_rnd(pkt.pts - offset_pts, in_stream->time_base,
            out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts - offset_pts, in_stream->time_base,
            out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base,
            out_stream->time_base);
        pkt.pos = -1;
        ret = av_interleaved_write_frame(ec, &pkt);
        if (ret != 0) {
            PrintError(ret);
            break;
        }
        av_packet_unref(&pkt);
    }
    //写入结尾, 包含文件偏移索引
    ret = av_write_trailer(ec);
    if (ret != 0) {
        PrintError(ret);
        return -1;
    }
    avformat_close_input(&ic);//与avformat_open_input成对使用
    //avio_closep(&ec->pb);
    avformat_free_context(ec);
    return 0;
}