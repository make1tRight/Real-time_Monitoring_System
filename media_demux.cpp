#include "media_demux.h"
#include "media_util.h"

AVFormatContext* MediaDemux::OpenContext(const char* url)
{
	AVFormatContext* context = nullptr;
	//�򿪷�װcontext
	auto ret = avformat_open_input(&context, url, nullptr, nullptr);
	if (ret != 0) {
		PrintError(ret);
		return nullptr;
	}
	//��ȡý����Ϣ
	ret = avformat_find_stream_info(context, nullptr);
	if (ret != 0) {
		PrintError(ret);
		return nullptr;
	}
	//��ӡ�����װ��Ϣ
	av_dump_format(context, 0, url, 0);
	return context;
}

bool MediaDemux::ReadPacket(AVPacket* pkt)
{
	std::lock_guard<std::mutex> lock(_mtx);
	if (_context == nullptr) {
		return false;
	}
	auto ret = av_read_frame(_context, pkt);
	if (ret != 0) {
		PrintError(ret);
		return false;
	}
	return true;
}

bool MediaDemux::SeekFrame(long long pts, int stream_index)
{
	std::lock_guard<std::mutex> lock(_mtx);
	if (_context == nullptr) {
		return false;
	}
	auto ret = av_seek_frame(_context, stream_index, pts,
		AVSEEK_FLAG_ANY|AVSEEK_FLAG_BACKWARD);//����ؼ�֡
	if (ret < 0) {
		PrintError(ret);
		return false;
	}
	return true;
}


