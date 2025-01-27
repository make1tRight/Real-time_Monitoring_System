#include <iostream>
#include <fstream>
#include "media_render.h"
#include "media_codec.h"
#include "media_encoder.h"
#include "media_util.h"
#include "media_decoder.h"


int main(int argc, char* argv[])
{
    //��ý���ļ�
    const char* url = "test2.mp4";
    //���װ����������
    AVFormatContext* ic = nullptr;
    auto ret = avformat_open_input(&ic, url,
        NULL,		//��װ����ʽ, NULL��ʾ���ݺ�׺���Զ����
        NULL		//rtsp��Ҫ����
    );
    if (ret != 0) {
        PrintError(ret);
        return -1;
    }
    //��ȡý����Ϣ(��ͷ����ʽ)
    ret = avformat_find_stream_info(ic, NULL);
    if (ret < 0) {
        PrintError(ret);
        return -1;
    }
    av_dump_format(ic, 0, url,
        0//is_output=0��ʾ����
    );
    AVStream* as = nullptr;//��Ƶ��
    AVStream* vs = nullptr;//��Ƶ��
    for (int i = 0; i < ic->nb_streams; ++i) {
        //�������Ƶ
        if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            as = ic->streams[i];
            std::cout << "========Audio========" << std::endl;
            std::cout << "sample_rate=" << as->codecpar->sample_rate << std::endl;
            std::cout << "========Audio========" << std::endl;
        }
        //�������Ƶ
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
        out_url);//�����ļ���ƥ���װ��ʽ
    if (ret < 0) {
        PrintError(ret);
        return -1;
    }
    //�����Ƶ������Ƶ��, mvs, mas������context�����ٶ�����
    auto mvs = avformat_new_stream(ec, NULL);//��Ƶ��
    auto mas = avformat_new_stream(ec, NULL);//��Ƶ��
    ret = avio_open(&ec->pb, out_url, AVIO_FLAG_WRITE);
    if (ret < 0) {
        PrintError(ret);
        return -1;
    }
    if (vs) {
        mvs->time_base = vs->time_base;//��ԭ��Ƶ��time_baseһ��
        //�ӽ��װ�и��Ʋ���
        avcodec_parameters_copy(mvs->codecpar, vs->codecpar);
    }
    if (as) {
        mas->time_base = as->time_base;
        avcodec_parameters_copy(mas->codecpar, as->codecpar);
    }
    //д���ļ�ͷ
    ret = avformat_write_header(ec, NULL);
    if (ret < 0) {
        PrintError(ret);
        return -1;
    }
    //��ӡ���������
    av_dump_format(ec, 0, out_url, 1);

    double begin_sec = 10.0;//��ȡ��ʼʱ��
    double end_sec = begin_sec + 20.0;	//��ȡ����ʱ��
    long long begin_pts = 0;//��Ƶ��ʼʱ��
    long long begin_audio_pts = 0;//��Ƶ��ʼʱ��
    long long end_pts = 0;

    //�����pts, ���������ic��pts, ����Ƶ��Ϊ׼
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
            begin_pts, AVSEEK_FLAG_ANY|AVSEEK_FLAG_BACKWARD);//����ؼ�֡
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
        long long offset_pts = 0;//ƫ��pts, ���ڽضϵĿ�ͷpts����
        if (vs && pkt.stream_index == vs->index) {
            std::cout << "Video: ";
            //������Ƶ
            if (pkt.pts < begin_pts - 10) {
                av_packet_unref(&pkt);
                continue;
            }
            if (pkt.pts > end_pts) {
                av_packet_unref(&pkt);//��֤���ڴ���Լ�ʱ�ͷ�
                break;
            }
            out_stream = ec->streams[0];
            offset_pts = begin_pts;
        }
        if (as && pkt.stream_index == as->index) {
            std::cout << "Audio: ";
            // �����Ƶ֡��ʱ���������Ƶ�Ľ���ʱ�䣬��������Ƶ֡
            if (pkt.pts < begin_pts - 10) {
                av_packet_unref(&pkt);
                continue;
            }
            out_stream = ec->streams[1];
            offset_pts = begin_audio_pts;
        }
        std::cout << pkt.pts << ": " << pkt.dts << ": " << pkt.size << std::endl;
        //���¼���pts, dts, duration
        //д������Ƶ֡, ������pkt
        //The operation is mathematically equivalent to `a* bq(����timebase) / cq(���timebase)`.
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
    //д���β, �����ļ�ƫ������
    ret = av_write_trailer(ec);
    if (ret != 0) {
        PrintError(ret);
        return -1;
    }
    avformat_close_input(&ic);//��avformat_open_input�ɶ�ʹ��
    //avio_closep(&ec->pb);
    avformat_free_context(ec);
    return 0;
}