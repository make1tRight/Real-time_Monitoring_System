#include "media_decoder.h"
#include "media_util.h"
#include <iostream>

bool MediaDecoder::SendPacket(const AVPacket* pkt)
{
	std::lock_guard<std::mutex> lock(_mtx);
	if (_context == nullptr) {
		return false;
	}
	auto ret = avcodec_send_packet(_context, pkt);
	if (ret != 0) {
		return false;
	}
	return true;
}

bool MediaDecoder::RecvFrame(AVFrame* frame)
{
	if (!frame) {
		std::cerr << "Error: Input frame pointer is null." << std::endl;
		return false;
	}
	std::lock_guard<std::mutex> lock(_mtx);
	if (_context == nullptr) {
		return false;
	}
	auto f = frame;
	if (_context->hw_device_ctx) {//如果是硬件加速
		f = av_frame_alloc();
		if (f == nullptr) {
			std::cerr << "Error: Failed to allocate temporary frame for hardware decoding." << std::endl;
			return false;
		}
	}
	auto ret = avcodec_receive_frame(_context, f);
	if (ret != 0) {
		PrintError(ret);
		if (_context->hw_device_ctx) {
			av_frame_free(&f);
		}
		return false;
	}
	//如果是硬件解码
	if (_context->hw_device_ctx) {
		//可将GPU解码完成的数据拷贝到CPU中
		ret = av_hwframe_transfer_data(frame, f, 0);
		av_frame_free(&f);
		if (ret != 0) {
			PrintError(ret);
			return false;
		}
	}
	return true;
}

std::vector<AVFrame*> MediaDecoder::GetCachePkt()
{
	std::vector<AVFrame*> res;
	std::lock_guard<std::mutex> lock(_mtx);
	if (_context == nullptr) {
		return res;
	}
	//取出缓存中的数据
	int ret = avcodec_send_packet(_context, NULL);
	while (ret >= 0) {
		auto frame = av_frame_alloc();
		//每次都会调用av_frame_unref, 空间会重新释放和申请
		ret = avcodec_receive_frame(_context, frame);
		if (ret < 0) {
			//防止内存泄漏
			av_frame_free(&frame);
			break;
		}
		res.push_back(frame);
	}
	return res;
}

bool MediaDecoder::InitHardWare(int type)
{
	std::lock_guard<std::mutex> lock(_mtx);
	if (_context == nullptr) {
		return false;
	}
	AVBufferRef* ctx = nullptr;          //硬件加速上下文
	auto ret = av_hwdevice_ctx_create(&ctx, (AVHWDeviceType)type, NULL, NULL, 0);
	if (ret != 0) {
		PrintError(ret);
		return false;
	}
	_context->hw_device_ctx = ctx;
	std::cout << "hardware acceleration:" << type << std::endl;
	return true;
}
