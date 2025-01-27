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
	/// ���������������
	/// </summary>
	/// <param name="codec_id">������ID, ��Ӧffmpeg</param>
	/// <param name="codec_type">1-encoder, 2-decoder</param>
	/// <returns></returns>
	static AVCodecContext* CreateContext(int codec_id, CodecType codec_type);

	/// <summary>
	/// ���ö���ı���������|�����Ĵ��ݵ�������|��Ҫ��֤�̰߳�ȫ
	/// </summary>
	/// <param name="context">context ����������, ���context_��Ϊnullptr, ��Ҫ��������Դ</param>
	void SetContext(AVCodecContext* context);

	/// <summary>
	/// ���ñ������|��Ҫ��֤�̰߳�ȫ
	/// </summary>
	/// <param name="key"></param>
	/// <param name="value"></param>
	/// <returns></returns>
	bool SetOpt(const char* key, const char* value);
	bool SetOpt(const char* key, int value);

	/// <summary>
	/// �򿪱�����������|�̰߳�ȫ
	/// </summary>
	bool OpenContext();

	/// <summary>
	/// ����AVCodecContext����AVFrame
	/// </summary>
	/// <returns></returns>
	AVFrame* CreateAVFrame();

	/// <summary>
	/// ������
	/// </summary>
	void Clear() {}
protected:
	AVCodecContext* _context;
	std::mutex _mtx;
};

