#include "media_format.h"
#include "media_util.h"

MediaFormat::MediaFormat()
    : _context(nullptr), _video_index(-1), _audio_index(-1),
    _video_time_base({1, 25}), _audio_time_base({1, 9000})
{

}

bool MediaFormat::CopyParameters(int stream_index, AVCodecParameters* dst)
{
    if (stream_index < 0) {
        return false;
    }
    std::lock_guard<std::mutex> lock(_mtx);
    if (_context == nullptr) {
        return false;
    }
    if (stream_index > _context->nb_streams) {
        return false;
    }
    auto ret = avcodec_parameters_copy(dst, _context->streams[stream_index]->codecpar);
    if (ret < 0) {
        PrintError(ret);
        return false;
    }
    return true;
}

void MediaFormat::SetContext(AVFormatContext* context)
{
    std::lock_guard<std::mutex> lock(_mtx);
    if (_context) {//����Ѿ���context, �����������
        if (_context->oformat) {//���������
            //pb���ڹ������������IO����, ����Ҫ�ǵ��ͷ�
            if (_context->pb) {
                avio_closep(&_context->pb);
            }
            avformat_free_context(_context);
        }
        else if (_context->iformat) {//����������
            avformat_close_input(&_context);
        }
        else {//����Ȳ�������Ҳ�������, ˵������δ��ȫ��ʼ����״̬, ֱ���ͷ�
            avformat_free_context(_context);
        }
    }
    _context = context;
    if (_context == nullptr) {
        return;
    }
    for (int i = 0; i < context->nb_streams; ++i) {
        //�������Ƶ
        if (context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            _audio_index = i;
            _audio_time_base.den = context->streams[i]->time_base.den;
            _audio_time_base.num = context->streams[i]->time_base.num;
        }
        //�������Ƶ
        if (context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            _video_index = i;
            _video_time_base.den = context->streams[i]->time_base.den;
            _video_time_base.num = context->streams[i]->time_base.num;
        }
    }
}

bool MediaFormat::RescaleTime(AVPacket* pkt, long long offset_pts, XRational time_base)
{
    std::lock_guard<std::mutex> lock(_mtx);
    if (_context == nullptr) {
        return false;
    }
    auto out_stream = _context->streams[pkt->stream_index];
    AVRational in_time_base;
    in_time_base.num = time_base.num;
    in_time_base.den = time_base.den;
    pkt->pts = av_rescale_q_rnd(pkt->pts - offset_pts, in_time_base,
        out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    pkt->dts = av_rescale_q_rnd(pkt->dts - offset_pts, in_time_base,
        out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    pkt->duration = av_rescale_q(pkt->duration, in_time_base,
        out_stream->time_base);
    pkt->pos = -1;
    return true;
}
