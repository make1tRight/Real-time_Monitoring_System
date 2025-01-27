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
	/// ���캯��
	/// </summary>
	MediaFormat();

	/// <summary>
	/// ���Ʋ���|�̰߳�ȫ
	/// </summary>
	/// <param name="stream_index">stream_index:��Ӧcontext_->streams�±�</param>
	/// <param name="dst">�������</param>
	/// <returns>�Ƿ�ɹ�</returns>
	bool CopyParameters(int stream_index, AVCodecParameters* dst);

	/// <summary>
	/// ����������, ���������ϴ����õ�ֵ, �������NULL�൱�ڹر�������|�̰߳�ȫ
	/// </summary>
	/// <param name="context">������</param>
	void SetContext(AVFormatContext* context);

	/// <summary>
	/// ��ȡ��Ƶ������
	/// </summary>
	/// <returns>��Ƶ������</returns>
	int GetAudioIndex() { return _audio_index; }

	/// <summary>
	/// ��ȡ��Ƶ������
	/// </summary>
	/// <returns>��Ƶ������</returns>
	int GetVideoIndex() { return _video_index; }

	/// <summary>
	/// ��ȡ��Ƶʱ�����
	/// </summary>
	/// <returns>��Ƶʱ�����</returns>
	XRational GetVideoTimeBase() { return _video_time_base; }

	/// <summary>
	/// ��ȡ��Ƶʱ�����
	/// </summary>
	/// <returns>��Ƶʱ�����</returns>
	XRational GetAudioTimeBase() { return _audio_time_base; }

	/// <summary>
	/// ����timebase����ʱ��
	/// </summary>
	/// <param name="pkt">���ݰ�</param>
	/// <param name="offset_pts">ptsƫ����</param>
	/// <param name="time_base">����ʱ�����</param>
	/// <returns>��Ƶ��Ƶʱ�����</returns>
	bool RescaleTime(AVPacket* pkt, long long offset_pts, XRational time_base);

protected:
	AVFormatContext* _context;
	std::mutex _mtx;
	int _video_index;
	int _audio_index;
	XRational _video_time_base;
	XRational _audio_time_base;
};

