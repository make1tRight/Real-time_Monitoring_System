#pragma once
#include "media_codec.h"
#include <vector>

struct AVPacket;

class MediaEncoder : public MediaCodec
{
public:

	~MediaEncoder();
	/// <summary>
	/// 编码数据|需要保证线程安全|数据发送给AVFrame进行编码, 编码完成后返回AVPacket
	/// </summary>
	/// <param name="frame">编码源数据存在frame中</param>
	/// <returns>返回编码完成的数据</returns>
	AVPacket* Encode(const AVFrame* frame);

	/// <summary>
	/// 返回所有编码缓冲中的AVPacket
	/// </summary>
	/// <returns></returns>
	std::vector<AVPacket*> GetCachePkt();
};

