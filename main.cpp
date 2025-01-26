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



    auto video_codec_id = vs->codecpar->codec_id;	//��Ƶ������ID
    auto video_par = vs->codecpar;					//��Ƶ�������
    /* ��Ƶ��������ʼ�� */
    MediaDecoder media_decoder;
    auto decode_c = MediaCodec::CreateContext(video_codec_id, MediaCodec::DECODER);
    //���װ����Ƶ����������ݸ�������
    avcodec_parameters_to_context(decode_c, video_par);
    //���õ���������, ��֤�̰߳�ȫ, �������decode_c���������ⲿʹ��
    media_decoder.SetContext(decode_c);
    if (media_decoder.OpenContext() == false) {
        std::cout << "decode Open failed..." << std::endl;
        return -1;
    }
    /* ������������Ŀռ� */
    auto frame = media_decoder.CreateAVFrame();
    /* ��Ⱦ�ĳ�ʼ�� */
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
            //������Ƶ
            if (media_decoder.SendPacket(&pkt)) {
                while (media_decoder.RecvFrame(frame)) {
                    std::cout << frame->pts << " " << std::endl;
                    view->PresentFrame(frame);//��Ⱦ��Ƶ
                }
            }
        }
        if (as && pkt.stream_index == as->index) {
            std::cout << "Audio: ";
        }
        std::cout << pkt.pts << ": " << pkt.dts << ": " << pkt.size << std::endl;
        av_packet_unref(&pkt);//��֤���ڴ���Լ�ʱ�ͷ�
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    av_frame_free(&frame);
    avformat_close_input(&ic);//��avformat_open_input�ɶ�ʹ��
    return 0;
}