#include <iostream>
#include <fstream>
#include "media_render.h"
#include "media_codec.h"
#include "media_encoder.h"
#include "media_util.h"
#include "media_decoder.h"


int main(int argc, char* argv[])
{
    //std::string filename = "400_300_25_preset";
    //AVCodecID codec_id = AV_CODEC_ID_H264;
    //if (argc > 1) {
    //    std::string codec = argv[1];
    //    if (codec == "h265" || codec == "hevc") {
    //        codec_id = AV_CODEC_ID_HEVC;
    //    }
    //}
    //if (codec_id == AV_CODEC_ID_H264) {
    //    filename += ".h264";
    //}
    //if (codec_id == AV_CODEC_ID_HEVC) {
    //    filename += ".h265";
    //}
    //std::ofstream ofs;
    //ofs.open(filename, std::ios::binary);
    //if (!ofs) {
    //    std::cerr << "failed to open file..." << std::endl;
    //    return -1;
    //}
    //MediaEncoder media_encoder;
    //auto c = media_encoder.CreateContext(codec_id, MediaEncoder::ENCODER);
    //c->width = 400;
    //c->height = 300;
    //media_encoder.SetContext(c);
    //media_encoder.SetOpt("crf", 18);//CRF�Ǻ㶨��������
    //media_encoder.OpenContext();
    //auto frame = media_encoder.CreateAVFrame();
    //if (frame == nullptr) {
    //    std::cerr << "CreateFrame failed..." << std::endl;
    //    return -1;
    //}
    //int count = 0; //д���ļ���֡��, SPS, PPS, IDR����һ֡��
    ////1s25֡, 250֡����10s
    //for (int i = 0; i < 500; ++i) {
    //    //����AVFrame����, ÿ֡���ݲ�ͬ
    //    //Y
    //    for (int y = 0; y < c->height; ++y) {
    //        for (int x = 0; x < c->width; ++x) {
    //            frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
    //        }
    //    }
    //    //UV
    //    for (int y = 0; y < c->height / 2; ++y) {
    //        for (int x = 0; x < c->width / 2; ++x) {
    //            frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
    //            frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
    //        }
    //    }
    //    frame->pts = i;//��time_baseΪ������λ��ʾ��ʱ��
    //    auto pkt = media_encoder.Encode(frame);
    //    if (pkt) {
    //        ++count;
    //        ofs.write((char*)pkt->data, pkt->size);
    //        av_packet_free(&pkt);
    //    }
    //}
    //auto pkts = media_encoder.GetCachePkt();
    //for (auto pkt : pkts) {
    //    ++count;
    //    ofs.write((char*)pkt->data, pkt->size);
    //    av_packet_free(&pkt);
    //}
    //ofs.close();
    //media_encoder.SetContext(nullptr);
    //std::cout << "encode count=" << count << std::endl;

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

    //MediaDecoder media_decoder;
    //auto c = media_decoder.CreateContext(codec_id, MediaCodec::DECODER);
    //media_decoder.SetContext(c);
    //media_decoder.InitHardWare();
    //media_decoder.OpenContext();

    //1. �ҽ�����
    auto codec = avcodec_find_decoder(codec_id);
    //2. ����������������
    auto c = avcodec_alloc_context3(codec);
    //c->thread_count = 16;
    //3. ��������
    avcodec_open2(c, NULL, NULL);
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
                //std::cout << pkt->size << " " << std::endl;
                //����packet�������߳�
                ret = avcodec_send_packet(c, pkt);
                if (ret < 0) {
                    break;
                }
                //��ȡ��֡��������
                while (ret >= 0) {
                    //ÿ�ζ������av_frame_unref, �ռ�������ͷź�����
                    ret = avcodec_receive_frame(c, frame);
                    if (ret < 0) {
                        break;
                    }
                    auto pframe = frame;
                    if (c->hw_device_ctx) { //Ӳ����
                        //Ӳ����ת��GPU->CPU(�Դ�->�ڴ�)
                        //AV_PIX_FMT_NV12 = 23,      < planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components,
                        //which are interleaved (first byte U and the following byte V)
                        av_hwframe_transfer_data(hw_frame, frame, 0);
                        pframe = hw_frame;
                    }
                    std::cout << frame->format << std::endl;
                    //std::cout << frame->format << std::endl;
                    /* ��һ֡��ʼ������ */
                    if (!is_init_win) {
                        view->Init(frame->width, frame->height, (AVPixelFormat)pframe->format);
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
    //ȡ�������е�����
    int ret = avcodec_send_packet(c, NULL);
    while (ret >= 0) {
        //ÿ�ζ������av_frame_unref, �ռ�������ͷź�����
        ret = avcodec_receive_frame(c, frame);
        if (ret < 0) {
            break;
        }
        std::cout << "receive from cache:" << frame->format << std::endl;
    }
    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    return 0;
}