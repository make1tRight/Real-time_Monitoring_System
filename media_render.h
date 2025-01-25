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
	/// ����Context
	/// </summary>
	/// <param name="type">��Ⱦ��������</param>
	/// <returns></returns>
	static MediaRender* CreateContext(RenderType type = SDL);

	/// <summary>
	/// ��ʼ����Ⱦ���� - �̰߳�ȫ|�ɶ�ε���
	/// </summary>
	/// <param name="width">���ڿ��</param>
	/// <param name="height">���ڸ߶�</param>
	/// <param name="fmt">���Ƶ����ظ�ʽ</param>
	/// <param name="win_id">���ھ��, ���Ϊ���򴴽��´���</param>
	/// <returns>�Ƿ񴴽��ɹ�</returns>
	/*virtual bool Init(int width, int height,
		SDL_PixelFormatEnum format = SDL_PIXELFORMAT_RGBA8888, void* win_id = nullptr) = 0;*/
	virtual bool Init(int width, int height,
		AVPixelFormat format = AV_PIX_FMT_YUV420P) = 0;

	/// <summary>
	/// ��Ⱦͼ�� - ������Ҫ��ܸ�|�̰߳�ȫ
	/// </summary>
	/// <param name="data">��Ⱦ�Ķ���������</param>
	/// <param name="linesize">һ�����ݵ��ֽ���, ����YUV420P����һ���ֽ���</param>
	/// <returns>�Ƿ���Ⱦ�ɹ�</returns>
	virtual bool Present(const char* data, int linesize = 0) = 0;

	virtual bool Present(
		const unsigned char* y, int y_pitch,
		const unsigned char* u, int u_pitch,
		const unsigned char* v, int v_pitch) = 0;

	bool PresentFrame(AVFrame* frame);

	/// <summary>
	/// ��ʾ����
	/// </summary>
	/// <param name="width">���ڿ��</param>
	/// <param name="height">���ڸ߶�</param>
	void SetShowSize(int width, int height);

	/// <summary>
	/// ���������������Դ, ���رմ���
	/// </summary>
	virtual void Close() = 0;

	/// <summary>
	/// �������˳��¼�
	/// </summary>
	/// <returns></returns>
	virtual bool IsExit() = 0;

	/// <summary>
	/// �߳�˯�ߺ���
	/// </summary>
	/// <param name="millisecond">��λΪ����</param>
	void SleepThread(unsigned int millisecond);

	/// <summary>
	/// ��ȡ����֡��
	/// </summary>
	/// <returns>0-300</returns>
	int GetRenderFps();

	/// <summary>
	/// ���²���֡��
	/// </summary>
	void UpdateRenderFps();

protected:
	int _width;						//���ʿ�
	int _height;					//���ʸ�
	SDL_PixelFormatEnum _format;	//���ظ�ʽ
	AVPixelFormat _av_format;		//--
	std::mutex _mtx;
	int _render_width;				//ʵʱ��Ⱦ���
	int _render_height;				//ʵʱ��Ⱦ�߶�
	int _render_fps;				//ʵʱ��Ⱦ֡��
	std::chrono::steady_clock::time_point _beg_time_ms;
	int _count = 0;					//ͳ����ʾ����
	void* _win_id = nullptr;		//���ھ��
};

