#include "media_render.h"
#include "sdl_render.h"
extern "C" {
#include <libavcodec/avcodec.h>
}

MediaRender::MediaRender()
{
}

MediaRender* MediaRender::CreateContext(RenderType type)
{
	switch (type)
	{
	case MediaRender::SDL:
		return new SdlRender();
		break;
	default:
		break;
	}
	return nullptr;
}

bool MediaRender::PresentFrame(AVFrame* frame)
{
	if (!frame || !frame->data[0]) {
		return false;
	}
	UpdateRenderFps();

	if (frame->format == AV_PIX_FMT_YUV420P) {
		return Present(
			frame->data[0], frame->linesize[0],	//Y
			frame->data[1], frame->linesize[1],	//U
			frame->data[2], frame->linesize[2]	//V
		);
	}
	if (frame->format == AV_PIX_FMT_BGRA ||
		frame->format == AV_PIX_FMT_ARGB ||
		frame->format == AV_PIX_FMT_RGBA) {
		return Present((char*)frame->data[0], frame->linesize[0]);
	}
	return true;
}

void MediaRender::SetShowSize(int width, int height)
{
	_render_width = width;
	_render_height = height;
}

void MediaRender::SleepThread(unsigned int millisecond)
{
	auto beg = std::chrono::steady_clock::now();
	for (int i = 0; i < millisecond; ++i) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - beg).count();
		if (duration >= millisecond) {
			return;
		}
	}
}

int MediaRender::GetRenderFps()
{
	return _render_fps;
}

void MediaRender::UpdateRenderFps()
{
	++_count;
	if (_beg_time_ms == std::chrono::steady_clock::time_point()) {
		_beg_time_ms = std::chrono::steady_clock::now();
	}
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - _beg_time_ms).count();
	if (duration >= 1000) { // 每秒计算一次 FPS
		_render_fps = _count;
		_count = 0;
		_beg_time_ms = std::chrono::steady_clock::now();  // 重置开始时间
	}
}
