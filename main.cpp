#include <iostream>
#include <fstream>
#include "media_render.h"
#include "media_codec.h"
#include "media_encoder.h"
#include "media_util.h"
#include "media_decoder.h"


int main(int argc, char* argv[])
{
    auto view = MediaRender::CreateContext();
    //1. �ָ�h264, ����AVPacket
    // ffmpeg -i test.mp4 -s 400x300 test.h264
    std::string filename = "test.h264";
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        return -1;
    }
    unsigned char inbuf[4096] = { 0 };
    AVCodecID codec_id = AV_CODEC_ID_H264;

    MediaDecoder media_decoder;
    auto c = media_decoder.CreateContext(codec_id, MediaCodec::DECODER);
    media_decoder.SetContext(c);
    media_decoder.InitHardWare();
    media_decoder.OpenContext();

    //�ָ�������
    auto parser = av_parser_init(codec_id);
    auto pkt = av_packet_alloc();
    auto frame = av_frame_alloc();
    auto hw_frame = av_frame_alloc();//����Ӳ�����ת��
    auto begin = GetCurrentMsTime();
    int count = 0;//����ͳ��
    bool is_init_win = false;
    while (!ifs.eof()) {
        //����inbuf��
        ifs.read((char*)inbuf, sizeof(inbuf));
        int data_size = ifs.gcount();
        if (data_size <= 0) {
            break;
        }
        //ѭ������
        if (ifs.eof()) {
            ifs.clear();
            ifs.seekg(0, std::ios::beg);
        }
        auto data = inbuf;
        while (data_size > 0) {	//һ���ж�֡������ѭ������ 
            //ͨ��0001 �ض�, �����AVPacket
            int ret = av_parser_parse2(parser, c,
                &pkt->data, &pkt->size,		//���
                data, data_size,			//����
                AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0
            );
            //����һ��λ�ü������д���
            data += ret;
            data_size -= ret;
            if (pkt->size) {
                //����packet�������߳�
                if (media_decoder.SendPacket(pkt) == false) {
                    break;
                }
                //��ȡ��֡��������
                while (media_decoder.RecvFrame(frame)) {
                    /* ��һ֡��ʼ������ */
                    if (!is_init_win) {
                        view->Init(frame->width, frame->height, (AVPixelFormat)frame->format);
                        is_init_win = true;
                    }
                     view->PresentFrame(frame);
                    ++count;
                    auto cur = GetCurrentMsTime();
                    if (cur - begin >= 100) {	//100ms����1��
                        std::cout << "fps=" << count * 10 << std::endl;
                        count = 0;
                        begin = cur;
                    }
                }
            }
        }
    }
    auto frames = media_decoder.GetCachePkt();
    for (auto& frame : frames) {
        view->PresentFrame(frame);
        av_frame_free(&frame);
    }
    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    return 0;
}