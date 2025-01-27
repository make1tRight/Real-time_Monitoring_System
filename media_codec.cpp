#include "media_codec.h"
#include "media_util.h"
#include <iostream>

//MediaCodec::MediaCodec(AVCodecContext* context)
//    : _context(context)
//{
//}

AVCodecContext* MediaCodec::CreateContext(int codec_id, CodecType codec_type)
{
    //1. 找到编码器
    //AVCodec不可变, 但是指针本身可以指向不同的对象
    const AVCodec* codec = nullptr;
    if (codec_type == ENCODER) {
        codec = avcodec_find_encoder((AVCodecID)codec_id);
    }
    if (codec_type == DECODER) {
        codec = avcodec_find_decoder((AVCodecID)codec_id);
    }
    if (!codec) {
        std::cerr << "avcodec_find_encoder failed, codec_id=" << codec_id << std::endl;
        return nullptr;
    }
    //2. 创建编码上下文
    auto c = avcodec_alloc_context3(codec);
    if (!c) {
        std::cerr << "avcodec_alloc_context3 failed, codec_id=" << codec_id << std::endl;
        return nullptr;
    }
    //设置参数默认值
    c->time_base = { 1, 25 };
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    c->thread_count = 16;//最好是根据计算机核心数来获取, 不要写常数
    return c;
}

void MediaCodec::SetContext(AVCodecContext* context)
{
    std::lock_guard<std::mutex> lock(_mtx);
    if (_context) {
        avcodec_free_context(&_context);
    }
    _context = context;
}

bool MediaCodec::SetOpt(const char* key, const char* value)
{
    std::lock_guard<std::mutex> lock(_mtx);
    if (!_context) {
        return false;
    }
    auto ret = av_opt_set(_context->priv_data, key, value, 0);
    if (ret != 0) {
        std::cerr << "av_opt_set failed..." << std::endl;
        PrintError(ret);
    }
    return true;
}

bool MediaCodec::SetOpt(const char* key, int value)
{
    std::lock_guard<std::mutex> lock(_mtx);
    if (!_context) {
        return false;
    }
    auto ret = av_opt_set_int(_context->priv_data, key, value, 0);
    if (ret != 0) {
        std::cerr << "av_opt_set failed..." << std::endl;
        PrintError(ret);
    }
    return true;
}

bool MediaCodec::OpenContext()
{
    std::lock_guard<std::mutex> lock(_mtx);
    if (!_context) {
        return false;
    }
    auto ret = avcodec_open2(_context, NULL, NULL);
    if (ret != 0) {
        PrintError(ret);
        return false;
    }
    return true;
}

AVFrame* MediaCodec::CreateAVFrame()
{
    std::lock_guard<std::mutex> lock(_mtx);
    if (_context == nullptr) {
        return nullptr;
    }
    auto frame = av_frame_alloc();
    frame->width = _context->width;
    frame->height = _context->height;
    frame->format = _context->pix_fmt;
    auto ret = av_frame_get_buffer(frame, 0);
    if (ret != 0) {
        av_frame_free(&frame);
        PrintError(ret);
        return nullptr;
    }
    return frame;
}


