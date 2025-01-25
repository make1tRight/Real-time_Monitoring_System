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
    //1. 分割h264, 存入AVPacket
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

    //分割上下文
    auto parser = av_parser_init(codec_id);
    auto pkt = av_packet_alloc();
    auto frame = av_frame_alloc();
    auto hw_frame = av_frame_alloc();//用于硬解码的转换
    auto begin = GetCurrentMsTime();
    int count = 0;//解码统计
    bool is_init_win = false;
    while (!ifs.eof()) {
        //读到inbuf中
        ifs.read((char*)inbuf, sizeof(inbuf));
        int data_size = ifs.gcount();
        if (data_size <= 0) {
            break;
        }
        //循环播放
        if (ifs.eof()) {
            ifs.clear();
            ifs.seekg(0, std::ios::beg);
        }
        auto data = inbuf;
        while (data_size > 0) {	//一次有多帧数据则循环调用 
            //通过0001 截断, 输出到AVPacket
            int ret = av_parser_parse2(parser, c,
                &pkt->data, &pkt->size,		//输出
                data, data_size,			//输入
                AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0
            );
            //从上一个位置继续进行处理
            data += ret;
            data_size -= ret;
            if (pkt->size) {
                //发送packet到解码线程
                if (media_decoder.SendPacket(pkt) == false) {
                    break;
                }
                //获取多帧解码数据
                while (media_decoder.RecvFrame(frame)) {
                    /* 第一帧初始化窗口 */
                    if (!is_init_win) {
                        view->Init(frame->width, frame->height, (AVPixelFormat)frame->format);
                        is_init_win = true;
                    }
                     view->PresentFrame(frame);
                    ++count;
                    auto cur = GetCurrentMsTime();
                    if (cur - begin >= 100) {	//100ms计算1次
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