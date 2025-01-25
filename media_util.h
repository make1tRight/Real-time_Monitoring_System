#pragma once
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
}
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")

/// <summary>
/// 打印错误消息
/// </summary>
/// <param name="error">错误代码</param>
void PrintError(int error);

/// <summary>
/// 获取当前毫秒数
/// </summary>
/// <returns></returns>
long long GetCurrentMsTime();

/// <summary>
/// 线程睡眠函数
/// </summary>
/// <param name="millisecond">单位为毫秒</param>
void SleepThread(unsigned int millisecond);


class MediaThread {
protected:

};