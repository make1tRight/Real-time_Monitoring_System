#include "media_util.h"
#include <iostream>
#include <chrono>
#include <thread>


void PrintError(int error)
{
	char buf[1024] = { 0 };
	av_strerror(error, buf, sizeof(buf) - 1);
	std::cerr << buf << std::endl;
}

//long long NowMs()
//{
//	return clock() / (CLOCKS_PER_SEC / 1000);
//}
long long GetCurrentMsTime()
{
	auto beg = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(beg.time_since_epoch()).count();
}

void SleepThread(unsigned int millisecond)
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
