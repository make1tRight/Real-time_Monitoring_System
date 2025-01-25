#pragma once
#include "media_codec.h"
#include <vector>

struct AVPacket;

class MediaDecoder : public MediaCodec
{
public:
	/// <summary>
	/// ���ݷ��͵������߳̽��н���
	/// </summary>
	/// <param name="pkt">������AVPacket�����ݸ�ʽ����</param>
	/// <returns></returns>
	bool SendPacket(const AVPacket* pkt);

	/// <summary>
	/// ���������ݷ��ص�frame��
	/// </summary>
	/// <param name="frame">���ڴ洢����������, ��ֱ����Ⱦ</param>
	/// <returns></returns>
	bool RecvFrame(AVFrame* frame);

	/// <summary>
	/// ��ȡ��������
	/// </summary>
	/// <returns></returns>
	std::vector<AVFrame*> GetCachePkt();

	/// <summary>
	/// Ӳ����ʼ��
	/// </summary>
	/// <param name="type">Ĭ����AV_HWDEVICE_TYPE_DXVA2</param>
	/// <returns></returns>
	bool InitHardWare(int type = 4);
};

