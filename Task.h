#pragma once
#include "Timer.h"
#include <string>
#include <fstream>
#include <thread>
#include <atomic>

namespace myTask {
	// 关键修复：将 myTimer::Timer 引入当前命名空间，否则编译器找不到 Timer
	using myTimer::Timer;

	struct Task {
		std::string m_name;
		std::string m_detail;
		int m_totalSeconds;
		Timer m_timer;
		std::thread m_timerThread;
		std::atomic<bool> m_started{ false };

		Task();
		Task(std::string name, std::string detail, int hour, int minute, int second);
		~Task();

		void start();
		void stop();
		void pause();
		void resume();
		void wait();
		int getRemaining() const;
		bool isFinished() const;
		void writeLog() const;
	};
}
