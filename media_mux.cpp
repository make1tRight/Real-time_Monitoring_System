#include "media_mux.h"
#include "media_util.h"

AVFormatContext* MediaMux::OpenContext(const char* url)
{
    AVFormatContext* c = nullptr;
    //创建上下文
    auto ret = avformat_alloc_output_context2(&c, NULL, NULL, url);//url的后缀名用于判断封装器的选择
    if (ret < 0) {
        PrintError(ret);
        return nullptr;
    }
    //添加视频音频流
    auto vs = avformat_new_stream(c, NULL);//视频流
    vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    auto as = avformat_new_stream(c, NULL);//音频流
    as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    //打开IO
    ret = avio_open(&c->pb, url, AVIO_FLAG_WRITE);//url用于输出
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
    //写入一帧数据, 内部缓冲排序dts, 通过传递pkt=nullptr 可以写入缓冲
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
    //发送空pkt, 写入排序缓冲区的数据
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
