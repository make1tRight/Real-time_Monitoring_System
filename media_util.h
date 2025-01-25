#pragma once
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
}
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")

/// <summary>
/// ��ӡ������Ϣ
/// </summary>
/// <param name="error">�������</param>
void PrintError(int error);

/// <summary>
/// ��ȡ��ǰ������
/// </summary>
/// <returns></returns>
long long GetCurrentMsTime();

/// <summary>
/// �߳�˯�ߺ���
/// </summary>
/// <param name="millisecond">��λΪ����</param>
void SleepThread(unsigned int millisecond);


class MediaThread {
protected:

};