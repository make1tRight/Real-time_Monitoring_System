#pragma once
#include "media_codec.h"
#include <vector>

struct AVPacket;

class MediaEncoder : public MediaCodec
{
public:

	~MediaEncoder();
	/// <summary>
	/// ��������|��Ҫ��֤�̰߳�ȫ|���ݷ��͸�AVFrame���б���, ������ɺ󷵻�AVPacket
	/// </summary>
	/// <param name="frame">����Դ���ݴ���frame��</param>
	/// <returns>���ر�����ɵ�����</returns>
	AVPacket* Encode(const AVFrame* frame);

	/// <summary>
	/// �������б��뻺���е�AVPacket
	/// </summary>
	/// <returns></returns>
	std::vector<AVPacket*> GetCachePkt();
};

