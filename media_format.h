#pragma once
#include <mutex>

struct AVCodecParameters;
struct AVFormatContext;
struct AVPacket;
struct XRational {
	int num; ///< Numerator
	int den; ///< Denominator
};

class MediaFormat
{
public:
	/// <summary>
	/// 构造函数
	/// </summary>
	MediaFormat();

	/// <summary>
	/// 复制参数|线程安全
	/// </summary>
	/// <param name="stream_index">stream_index:对应context_->streams下标</param>
	/// <param name="dst">输出参数</param>
	/// <returns>是否成功</returns>
	bool CopyParameters(int stream_index, AVCodecParameters* dst);

	/// <summary>
	/// 设置上下文, 并且清理上次设置的值, 如果传递NULL相当于关闭上下文|线程安全
	/// </summary>
	/// <param name="context">上下文</param>
	void SetContext(AVFormatContext* context);

	/// <summary>
	/// 获取音频流索引
	/// </summary>
	/// <returns>音频流索引</returns>
	int GetAudioIndex() { return _audio_index; }

	/// <summary>
	/// 获取视频流索引
	/// </summary>
	/// <returns>视频流索引</returns>
	int GetVideoIndex() { return _video_index; }

	/// <summary>
	/// 获取视频时间基数
	/// </summary>
	/// <returns>视频时间基数</returns>
	XRational GetVideoTimeBase() { return _video_time_base; }

	/// <summary>
	/// 获取音频时间基数
	/// </summary>
	/// <returns>音频时间基数</returns>
	XRational GetAudioTimeBase() { return _audio_time_base; }

	/// <summary>
	/// 根据timebase换算时间
	/// </summary>
	/// <param name="pkt">数据包</param>
	/// <param name="offset_pts">pts偏移量</param>
	/// <param name="time_base">输入时间基数</param>
	/// <returns>音频视频时间基数</returns>
	bool RescaleTime(AVPacket* pkt, long long offset_pts, XRational time_base);

protected:
	AVFormatContext* _context;
	std::mutex _mtx;
	int _video_index;
	int _audio_index;
	XRational _video_time_base;
	XRational _audio_time_base;
};

