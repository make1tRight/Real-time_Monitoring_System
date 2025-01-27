#pragma once
#include "media_format.h"
class MediaDemux : public MediaFormat
{
public:
	/// <summary>
	/// 打开解封装上下文
	/// </summary>
	/// <param name="url">文件或数据流</param>
	/// <returns>失败返回nullptr</returns>
	static AVFormatContext* OpenContext(const char* url);

	/// <summary>
	/// 读取数据包
	/// </summary>
	/// <param name="pkt">输出数据</param>
	/// <returns>是否成功</returns>
	bool ReadPacket(AVPacket* pkt);

	/// <summary>
	/// 移动到关键帧的位置
	/// </summary>
	/// <param name="pts">presentation timestamp</param>
	/// <param name="stream_index">stream_index: 关键帧的位置</param>
	/// <returns>是否成功</returns>
	bool SeekFrame(long long pts, int stream_index);
};

