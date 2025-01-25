#pragma once
#include "media_codec.h"
#include <vector>

struct AVPacket;

class MediaDecoder : public MediaCodec
{
public:
	/// <summary>
	/// 数据发送到解码线程进行解码
	/// </summary>
	/// <param name="pkt">数据以AVPacket的数据格式发送</param>
	/// <returns></returns>
	bool SendPacket(const AVPacket* pkt);

	/// <summary>
	/// 解码后的数据返回到frame中
	/// </summary>
	/// <param name="frame">用于存储解码后的数据, 可直接渲染</param>
	/// <returns></returns>
	bool RecvFrame(AVFrame* frame);

	/// <summary>
	/// 获取缓存数据
	/// </summary>
	/// <returns></returns>
	std::vector<AVFrame*> GetCachePkt();

	/// <summary>
	/// 硬件初始化
	/// </summary>
	/// <param name="type">默认是AV_HWDEVICE_TYPE_DXVA2</param>
	/// <returns></returns>
	bool InitHardWare(int type = 4);
};

