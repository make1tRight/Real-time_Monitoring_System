#pragma once
#include <mutex>

struct AVCodecContext;
struct AVFrame;

class MediaCodec
{
public:
	enum CodecType {
		DECODER = 0,
		ENCODER = 1
	};

	//MediaCodec(AVCodecContext* context = nullptr);
	/// <summary>
	/// 创建编解码上下文
	/// </summary>
	/// <param name="codec_id">编码器ID, 对应ffmpeg</param>
	/// <param name="codec_type">1-encoder, 2-decoder</param>
	/// <returns></returns>
	static AVCodecContext* CreateContext(int codec_id, CodecType codec_type);

	/// <summary>
	/// 设置对象的编码上下文|上下文传递到对象中|需要保证线程安全
	/// </summary>
	/// <param name="context">context 编码上下文, 如果context_不为nullptr, 需要先清理资源</param>
	void SetContext(AVCodecContext* context);

	/// <summary>
	/// 设置编码参数|需要保证线程安全
	/// </summary>
	/// <param name="key"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	bool SetOpt(const char* key, const char* value);
	bool SetOpt(const char* key, int value);

	/// <summary>
	/// 打开编码器上下文|线程安全
	/// </summary>
	bool OpenContext();

	/// <summary>
	/// 根据AVCodecContext创建AVFrame
	/// </summary>
	/// <returns></returns>
	AVFrame* CreateAVFrame();

	/// <summary>
	/// 清理缓存
	/// </summary>
	void Clear() {}
protected:
	AVCodecContext* _context;
	std::mutex _mtx;
};

