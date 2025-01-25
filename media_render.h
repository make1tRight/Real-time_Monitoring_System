#pragma once
#include <mutex>
#include <chrono>
#include <SDL/SDL.h>
#include "media_util.h"
#pragma comment(lib, "SDL2.lib")
#undef main

struct AVFrame;

class MediaRender
{
public:
	MediaRender();

	enum RenderType {
		SDL = 0
	};

	/// <summary>
	/// 创建Context
	/// </summary>
	/// <param name="type">渲染工具类型</param>
	/// <returns></returns>
	static MediaRender* CreateContext(RenderType type = SDL);

	/// <summary>
	/// 初始化渲染窗口 - 线程安全|可多次调用
	/// </summary>
	/// <param name="width">窗口宽度</param>
	/// <param name="height">窗口高度</param>
	/// <param name="fmt">绘制的像素格式</param>
	/// <param name="win_id">窗口句柄, 如果为空则创建新窗口</param>
	/// <returns>是否创建成功</returns>
	/*virtual bool Init(int width, int height,
		SDL_PixelFormatEnum format = SDL_PIXELFORMAT_RGBA8888, void* win_id = nullptr) = 0;*/
	virtual bool Init(int width, int height,
		AVPixelFormat format = AV_PIX_FMT_YUV420P) = 0;

	/// <summary>
	/// 渲染图像 - 对性能要求很高|线程安全
	/// </summary>
	/// <param name="data">渲染的二进制数据</param>
	/// <param name="linesize">一行数据的字节数, 对于YUV420P就是一行字节数</param>
	/// <returns>是否渲染成功</returns>
	virtual bool Present(const char* data, int linesize = 0) = 0;

	virtual bool Present(
		const unsigned char* y, int y_pitch,
		const unsigned char* u, int u_pitch,
		const unsigned char* v, int v_pitch) = 0;

	bool PresentFrame(AVFrame* frame);

	/// <summary>
	/// 显示缩放
	/// </summary>
	/// <param name="width">窗口宽度</param>
	/// <param name="height">窗口高度</param>
	void SetShowSize(int width, int height);

	/// <summary>
	/// 清理所有申请的资源, 并关闭窗口
	/// </summary>
	virtual void Close() = 0;

	/// <summary>
	/// 处理窗口退出事件
	/// </summary>
	/// <returns></returns>
	virtual bool IsExit() = 0;

	/// <summary>
	/// 线程睡眠函数
	/// </summary>
	/// <param name="millisecond">单位为毫秒</param>
	void SleepThread(unsigned int millisecond);

	/// <summary>
	/// 获取播放帧率
	/// </summary>
	/// <returns>0-300</returns>
	int GetRenderFps();

	/// <summary>
	/// 更新播放帧率
	/// </summary>
	void UpdateRenderFps();

protected:
	int _width;						//材质宽
	int _height;					//材质高
	SDL_PixelFormatEnum _format;	//像素格式
	AVPixelFormat _av_format;		//--
	std::mutex _mtx;
	int _render_width;				//实时渲染宽度
	int _render_height;				//实时渲染高度
	int _render_fps;				//实时渲染帧率
	std::chrono::steady_clock::time_point _beg_time_ms;
	int _count = 0;					//统计显示次数
	void* _win_id = nullptr;		//窗口句柄
};

