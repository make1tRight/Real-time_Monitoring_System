#include "media_encoder.h"
#include "media_util.h"
#include <string.h>

MediaEncoder::~MediaEncoder()
{

}

AVPacket* MediaEncoder::Encode(const AVFrame* frame)
{
    if (frame == nullptr) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(_mtx);
    if (_context == nullptr) {
        return nullptr;
    }
    //发送到编码线程
    auto ret = avcodec_send_frame(_context, frame);
    if (ret != 0) {
        return nullptr;
    }
    auto pkt = av_packet_alloc();

    //接收编码线程的数据
    ret = avcodec_receive_packet(_context, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        //这里收不到是正常的, 因为数据不是每时每刻都准备好的
        av_packet_free(&pkt);
        return nullptr;
    }
    if (ret < 0) {
        av_packet_free(&pkt);
        PrintError(ret);
        return nullptr;
    }
    return pkt;
}

std::vector<AVPacket*> MediaEncoder::GetCachePkt()
{
    std::vector<AVPacket*> res;
    std::lock_guard<std::mutex> lock(_mtx);
    if (_context == nullptr) {
        return res;
    }
    auto ret = avcodec_send_frame(_context, NULL);//发送NULL刷新编码器,获取缓冲区中的剩余数据
    while (ret != 0) {
        auto pkt = av_packet_alloc();
        ret = avcodec_receive_packet(_context, pkt);
        if (ret != 0) {
            av_packet_free(&pkt);
            break;
        }
        res.push_back(pkt);
    }
    return res;
}
