#include "sdl_render.h"
#include <iostream>
bool SdlRender::Init(int width, int height, AVPixelFormat format)
{
	if (width <= 0 || height <= 0) {
		return false;
	}
	//1. ��ʼ��SDL��Ƶ��
	InitVideo();
	std::lock_guard<std::mutex> lock(_mtx);
	_width = width;
	_height = height;
	_av_format = format;
	if (_av_format == AV_PIX_FMT_YUV420P) {
		_format = SDL_PIXELFORMAT_IYUV;
	}
	//AV_PIX_FMT_ARGB,      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
	//AV_PIX_FMT_RGBA,      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
	//AV_PIX_FMT_ABGR,      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
	//AV_PIX_FMT_BGRA,      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
	if (_av_format == AV_PIX_FMT_RGBA) {
		_format = SDL_PIXELFORMAT_RGBA32;
	}
	if (_av_format == AV_PIX_FMT_BGRA) {
		_format = SDL_PIXELFORMAT_BGRA32;
	}
	if (_av_format == AV_PIX_FMT_ARGB) {
		_format = SDL_PIXELFORMAT_ARGB32;
	}
	if (_av_format == AV_PIX_FMT_NV12) {
		_format = SDL_PIXELFORMAT_NV12;
	}
	//����֮ǰ������, ʵ�ֿɶ�ε���
	if (_texture) {
		SDL_DestroyTexture(_texture);
	}
	if (_renderer) {
		SDL_DestroyRenderer(_renderer);
	}
	// 2. �½�����
	if (!_win) {
		if (!_win_id) {
			_win = SDL_CreateWindow("",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				width, height,
				SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		}
		if (_win_id) {
			//��Ⱦ���ؼ�����
			_win = SDL_CreateWindowFrom(_win_id);
		}
	}
	//���û�д����ɹ��Ļ����������Ϣ
	if (!_win) {
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
	// 3. ������Ⱦ��
	_renderer = SDL_CreateRenderer(_win, -1, SDL_RENDERER_ACCELERATED);
	if (!_renderer) {
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
	//4. ���ɲ���
	//����IYUV֧����ʾyuv�ĸ�ʽ
	_texture = SDL_CreateTexture(_renderer,
		_format,
		SDL_TEXTUREACCESS_STREAMING,
		width, height
	);
	if (!_texture) {
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
	return true;
}

bool SdlRender::Present(const char* data, int linesize)
{
	if (!data) {
		return false;
	}
	std::lock_guard<std::mutex> lock(_mtx);
	if (!_texture || !_renderer || !_win || _width <= 0 || _height <= 0) {
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
	if (linesize <= 0) {
		//if (_format == SDL_PIXELFORMAT_RGBA8888 ||
		//	_format == SDL_PIXELFORMAT_ARGB8888) {
		//	linesize = _width * 4;
		//}
		//if (_format == SDL_PIXELFORMAT_IYUV) {
		//	linesize = _width;
		//}
		if (_av_format == AV_PIX_FMT_YUV420P) {
			linesize = _width;
		}
	}
	if (linesize <= 0) {
		std::cerr << "linesize <= 0" << std::endl;
		return false;
	}
	//5. �����ڴ����ݵ��Դ���
	auto ret = SDL_UpdateTexture(_texture, NULL, data, linesize);
	if (ret != 0) {
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
	// 6.�����Ļ
	SDL_RenderClear(_renderer);
	//7. ���Ʋ��ʵ���Ⱦ��
	if (_render_width > 0 || _render_height > 0) {
		SDL_Rect sdl_rect;
		sdl_rect.x = 0;
		sdl_rect.y = 0;
		sdl_rect.w = _render_width;
		sdl_rect.h = _render_height;
		ret = SDL_RenderCopy(_renderer, _texture, NULL, &sdl_rect);
	}
	else {
		ret = SDL_RenderCopy(_renderer, _texture, NULL, NULL);
	}
	if (ret != 0) {
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
	//8. ��Ⱦ
	SDL_RenderPresent(_renderer);
	return true;
}

bool SdlRender::Present(const unsigned char* y, int y_pitch,
	const unsigned char* u, int u_pitch,
	const unsigned char* v, int v_pitch)
{
	if (!y || !u || !v) {
		return false;
	}
	if (y_pitch <= 0 || u_pitch <= 0 || v_pitch <= 0) {
		return false;
	}
	std::lock_guard<std::mutex> lock(_mtx);
	if (!_texture || !_renderer || !_win || _width <= 0 || _height <= 0) {
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
	//5. �����ڴ����ݵ��Դ���
	auto ret = SDL_UpdateYUVTexture(_texture, NULL,
		y, y_pitch,
		u, u_pitch,
		v, v_pitch);
	if (ret != 0) {
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
	// 6.�����Ļ
	ret = SDL_RenderClear(_renderer);
	if (ret != 0) {
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
	//7. ���Ʋ��ʵ���Ⱦ��
	if (_render_width > 0 || _render_height > 0) {
		SDL_Rect sdl_rect;
		sdl_rect.x = 0;
		sdl_rect.y = 0;
		sdl_rect.w = _render_width;
		sdl_rect.h = _render_height;
		ret = SDL_RenderCopy(_renderer, _texture, NULL, &sdl_rect);
	}
	else {
		ret = SDL_RenderCopy(_renderer, _texture, NULL, NULL);
	}
	if (ret != 0) {
		std::cerr << SDL_GetError() << std::endl;
		return false;
	}
	//8. ��Ⱦ
	SDL_RenderPresent(_renderer);
	return true;
}

void SdlRender::Close()
{
	std::lock_guard<std::mutex> lock(_mtx);
	//�����ͷ���Դ
	if (_texture) {
		SDL_DestroyTexture(_texture);
		_texture = nullptr;
	}
	if (_renderer) {
		SDL_DestroyRenderer(_renderer);
		_renderer = nullptr;
	}
	if (_win) {
		SDL_DestroyWindow(_win);
		_win = nullptr;
	}
}

bool SdlRender::IsExit()
{
	SDL_Event event;
	SDL_WaitEventTimeout(&event, 1);
	if (event.type == SDL_QUIT) {
		return true;
	}
	return false;
}

bool SdlRender::InitVideo()
{
	static std::once_flag first_init;
	std::lock_guard<std::mutex> lock(_mtx);
	std::call_once(first_init, []() {
		auto ret = SDL_Init(SDL_INIT_VIDEO);
		if (ret < 0) {
			std::cout << SDL_GetError() << std::endl;
			return false;
		}
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	});
	return true;
}
