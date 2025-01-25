#pragma once
#include "media_render.h"
#include "media_util.h"

class SdlRender : public MediaRender
{
public:
	virtual bool Init(int width, int height,
		SDL_PixelFormatEnum format = SDL_PIXELFORMAT_RGBA8888, void* win_id = nullptr) override;
	virtual bool Init(int width, int height,
		AVPixelFormat format = AV_PIX_FMT_YUV420P) override;
	virtual bool Present(const char* data, int linesize = 0) override;
	virtual bool Present(
		const unsigned char* y, int y_pitch,
		const unsigned char* u, int u_pitch,
		const unsigned char* v, int v_pitch) override;
	virtual void Close() override;
	virtual bool IsExit() override;
	bool InitVideo();
private:
	SDL_Window* _win;
	SDL_Renderer* _renderer;
	SDL_Texture* _texture;
};

