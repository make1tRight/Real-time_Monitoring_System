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
    //���͵������߳�
    auto ret = avcodec_send_frame(_context, frame);
    if (ret != 0) {
        return nullptr;
    }
    auto pkt = av_packet_alloc();

    //���ձ����̵߳�����
    ret = avcodec_receive_packet(_context, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        //�����ղ�����������, ��Ϊ���ݲ���ÿʱÿ�̶�׼���õ�
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
    auto ret = avcodec_send_frame(_context, NULL);//����NULLˢ�±�����,��ȡ�������е�ʣ������
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
