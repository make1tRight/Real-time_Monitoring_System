#include <iostream>
#include <fstream>
#include "media_render.h"
#include "media_codec.h"
#include "media_encoder.h"
#include "media_util.h"
#include "media_decoder.h"
#include "media_demux.h"
#include "media_mux.h"


int main(int argc, char* argv[])
{
    //打开媒体文件
    std::string useage = "124_test_xformat|输入文件|输出文件|开始时间(s)|结束时间(s)\n";
    useage += "124_test_xformat test2.mp4 out_250127.mp4 10 20";
    std::cout << useage << std::endl;
    if (argc < 3) {
        return -1;
    }
    std::string in_file = argv[1];
    std::string out_file = argv[2];
    int begin_sec = 0;//截取开始时间
    int end_sec = 0;	//截取结束时间
    if (argc > 3) {
        begin_sec = atoi(argv[3]);
    }
    if (argc > 4) {
        end_sec = atoi(argv[4]);
    }
    //解封装输入上下文
    MediaDemux media_demux;
    auto demux_context = media_demux.OpenContext(in_file.c_str());
    media_demux.SetContext(demux_context);

    //封装上下文
    MediaMux media_mux;
    auto mux_context = media_mux.OpenContext(out_file.c_str());
    media_mux.SetContext(mux_context);
    auto mux_vs = mux_context->streams[media_mux.GetVideoIndex()];
    auto mux_as = mux_context->streams[media_mux.GetAudioIndex()];
    //如果有视频流
    if (media_demux.GetVideoIndex() == 0) {
        mux_vs->time_base.num = media_demux.GetVideoTimeBase().num;
        mux_vs->time_base.den = media_demux.GetVideoTimeBase().den;
        //复制视频参数
        media_demux.CopyParameters(media_demux.GetVideoIndex(), mux_vs->codecpar);
    }
    //如果有视频流
    if (media_demux.GetAudioIndex() == 1) {
        mux_as->time_base.num = media_demux.GetAudioTimeBase().num;
        mux_as->time_base.den = media_demux.GetAudioTimeBase().den;
        //复制视频参数
        media_demux.CopyParameters(media_demux.GetAudioIndex(), mux_as->codecpar);
    }
    media_mux.WriteHeader();

    long long video_begin_pts = 0;//视频开始时间
    long long audio_begin_pts = 0;//音频开始时间
    long long end_pts = 0;
    //换算成pts, 换算成输入ic的pts, 以视频流为准
    if (begin_sec > 0) {
        if (media_demux.GetVideoIndex() == 0 &&
            media_demux.GetVideoTimeBase().num > 0) {
            double t = (double)media_demux.GetVideoTimeBase().den /
                (double)media_demux.GetVideoTimeBase().num;
            video_begin_pts = (long long)(begin_sec * t);
            end_pts = (long long)(end_sec * t);
            //移动到开始帧
            media_demux.SeekFrame(video_begin_pts, media_demux.GetVideoIndex());
        }
        if (media_demux.GetAudioIndex() == 1 &&
            media_demux.GetAudioTimeBase().num > 0) {
            double t = (double)media_demux.GetAudioTimeBase().den /
                (double)media_demux.GetAudioTimeBase().num;
            audio_begin_pts = (long long)(begin_sec * t);
        }
    }

    int audio_count = 0;
    int video_count = 0;
    double total_sec = 0;
    AVPacket pkt;
    for (;;) {
        if (end_pts <= 0) {
            break;
        }
        if (media_demux.ReadPacket(&pkt) == false) {
            break; 
        }
        if (pkt.stream_index == media_demux.GetVideoIndex() &&
            pkt.pts < video_begin_pts - 10) {
            av_packet_unref(&pkt);
            continue;
        }
        if (pkt.stream_index == media_demux.GetVideoIndex() &&
            pkt.pts > end_pts) {
            av_packet_unref(&pkt);
            break;
        }
        if (pkt.stream_index == media_demux.GetVideoIndex()) {
            media_mux.RescaleTime(&pkt, video_begin_pts, media_demux.GetVideoTimeBase());
            ++video_count;
            if (media_demux.GetVideoTimeBase().den > 0) {
                total_sec += pkt.duration * ((double)media_demux.GetVideoTimeBase().num /
                    (double)media_demux.GetVideoTimeBase().den);
            }
        }
        if (pkt.stream_index == media_demux.GetAudioIndex()) {
            media_mux.RescaleTime(&pkt, audio_begin_pts, media_demux.GetAudioTimeBase());
            ++audio_count;
        }
        media_mux.WriteFrameFromPkt(&pkt);
    }
    //写入结尾, 包含文件偏移索引
    //ret = av_write_trailer(ec);
    media_mux.WriteTrailer();
    media_demux.SetContext(nullptr);
    media_mux.SetContext(nullptr);
    std::cout << "out_file:" << out_file << ":" << std::endl;
    std::cout << "video_count:" << video_count << std::endl;
    std::cout << "audio_count:" << audio_count << std::endl;
    std::cout << "total_sec:" << total_sec << std::endl;
    return 0;
}