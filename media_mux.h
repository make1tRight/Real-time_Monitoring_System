#pragma once
#include "media_format.h"

class MediaMux : public MediaFormat
{
public:
	/// <summary>
	/// 打开封装上下文
	/// </summary>
	/// <param name="url">文件或音视频流</param>
	/// <returns>context</returns>
	static AVFormatContext* OpenContext(const char* url);

	/// <summary>
	/// 写入头部信息
	/// </summary>
	/// <returns>是否成功</returns>
	bool WriteHeader();
	
	/// <summary>
	/// 写入音视频帧
	/// </summary>
	/// <param name="pkt">从音视频数据包中获取数据并写入</param>
	/// <returns>是否成功</returns>
	bool WriteFrameFromPkt(AVPacket* pkt);

	/// <summary>
	/// 写入结尾数据
	/// </summary>
	/// <returns>是否成功</returns>
	bool WriteTrailer();
};

