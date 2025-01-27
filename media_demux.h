#pragma once
#include "media_format.h"
class MediaDemux : public MediaFormat
{
public:
	/// <summary>
	/// �򿪽��װ������
	/// </summary>
	/// <param name="url">�ļ���������</param>
	/// <returns>ʧ�ܷ���nullptr</returns>
	static AVFormatContext* OpenContext(const char* url);

	/// <summary>
	/// ��ȡ���ݰ�
	/// </summary>
	/// <param name="pkt">�������</param>
	/// <returns>�Ƿ�ɹ�</returns>
	bool ReadPacket(AVPacket* pkt);

	/// <summary>
	/// �ƶ����ؼ�֡��λ��
	/// </summary>
	/// <param name="pts">presentation timestamp</param>
	/// <param name="stream_index">stream_index: �ؼ�֡��λ��</param>
	/// <returns>�Ƿ�ɹ�</returns>
	bool SeekFrame(long long pts, int stream_index);
};

