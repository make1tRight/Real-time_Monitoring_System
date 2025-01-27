#pragma once
#include "media_format.h"

class MediaMux : public MediaFormat
{
public:
	/// <summary>
	/// �򿪷�װ������
	/// </summary>
	/// <param name="url">�ļ�������Ƶ��</param>
	/// <returns>context</returns>
	static AVFormatContext* OpenContext(const char* url);

	/// <summary>
	/// д��ͷ����Ϣ
	/// </summary>
	/// <returns>�Ƿ�ɹ�</returns>
	bool WriteHeader();
	
	/// <summary>
	/// д������Ƶ֡
	/// </summary>
	/// <param name="pkt">������Ƶ���ݰ��л�ȡ���ݲ�д��</param>
	/// <returns>�Ƿ�ɹ�</returns>
	bool WriteFrameFromPkt(AVPacket* pkt);

	/// <summary>
	/// д���β����
	/// </summary>
	/// <returns>�Ƿ�ɹ�</returns>
	bool WriteTrailer();
};

