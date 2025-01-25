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
	if (_context->hw_device_ctx) {//�����Ӳ������
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
	//�����Ӳ������
	if (_context->hw_device_ctx) {
		//�ɽ�GPU������ɵ����ݿ�����CPU��
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
	//ȡ�������е�����
	int ret = avcodec_send_packet(_context, NULL);
	while (ret >= 0) {
		auto frame = av_frame_alloc();
		//ÿ�ζ������av_frame_unref, �ռ�������ͷź�����
		ret = avcodec_receive_frame(_context, frame);
		if (ret < 0) {
			//��ֹ�ڴ�й©
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
	AVBufferRef* ctx = nullptr;          //Ӳ������������
	auto ret = av_hwdevice_ctx_create(&ctx, (AVHWDeviceType)type, NULL, NULL, 0);
	if (ret != 0) {
		PrintError(ret);
		return false;
	}
	_context->hw_device_ctx = ctx;
	std::cout << "hardware acceleration:" << type << std::endl;
	return true;
}
