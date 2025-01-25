#pragma once
#include "media_codec.h"
#include <vector>

struct AVPacket;

class MediaDecoder : public MediaCodec
{
public:
	bool SendPacket(const AVPacket* pkt);
	bool RecvFrame(AVFrame* frame);
	std::vector<AVFrame*> GetCachePkt();
	bool InitHardWare(int type = 4);
};

