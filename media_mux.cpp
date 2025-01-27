#include "media_mux.h"
#include "media_util.h"

AVFormatContext* MediaMux::OpenContext(const char* url)
{
    AVFormatContext* c = nullptr;
    //����������
    auto ret = avformat_alloc_output_context2(&c, NULL, NULL, url);//url�ĺ�׺�������жϷ�װ����ѡ��
    if (ret < 0) {
        PrintError(ret);
        return nullptr;
    }
    //�����Ƶ��Ƶ��
    auto vs = avformat_new_stream(c, NULL);//��Ƶ��
    vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    auto as = avformat_new_stream(c, NULL);//��Ƶ��
    as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    //��IO
    ret = avio_open(&c->pb, url, AVIO_FLAG_WRITE);//url�������
    if (ret < 0) {
        PrintError(ret);
        return nullptr;
    }
    return c;
}

bool MediaMux::WriteHeader()
{
    std::lock_guard<std::mutex> lock(_mtx);
    if (_context == nullptr) {
        return false;
    }
    auto ret = avformat_write_header(_context, nullptr);
    if (ret < 0) {
        PrintError(ret);
        return false;
    }
    av_dump_format(_context, 0, _context->url, 1);
    return true;
}

bool MediaMux::WriteFrameFromPkt(AVPacket* pkt)
{
    if (pkt == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(_mtx);
    if (_context == nullptr) {
        return false;
    }
    //д��һ֡����, �ڲ���������dts, ͨ������pkt=nullptr ����д�뻺��
    auto ret = av_interleaved_write_frame(_context, pkt);
    if (ret != 0) {
        PrintError(ret);
        return false;
    }
    return true;
}

bool MediaMux::WriteTrailer()
{
    std::lock_guard<std::mutex> lock(_mtx);
    if (_context == nullptr) {
        return false;
    }
    //���Ϳ�pkt, д�����򻺳���������
    auto ret = av_interleaved_write_frame(_context, nullptr);
    if (ret != 0) {
        PrintError(ret);
        return false;
    }
    ret = av_write_trailer(_context);
    if (ret != 0) {
        PrintError(ret);
        return false;
    }
    return true;
}
