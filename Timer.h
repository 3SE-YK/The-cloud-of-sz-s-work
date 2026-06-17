#pragma once
#include<atomic>
#include<thread>
#include<chrono>
#include<iostream>
#include<mutex>
#include <condition_variable>
//计时器
namespace myTimer {
	class Timer
	{
	public:
		Timer(int period );
		~Timer();
		//启动定时任务
		void start();//这里我设计的计时器进需要进行固定的任务，没必要实现可编程功能
		void kick_off();//暂停计时
		void kick_on();//恢复计时
		void print_m_period()const { std::cout << m_period.load() / 3600 << ":" << m_period.load() / 60 % 60 << ":" << m_period.load() % 60 << '\n'; }
		void print_m_pause()const { std::cout << m_pause.load() << '\n'; }
		void print_m_running()const { std::cout << m_running.load() << '\n'; }
		int getPeriod() const { return m_period.load(); }
		void join_thread() { m_thread.join(); }
		void stop();
		void reset(int period);
	private:
		std::atomic<int> m_period{ 60 };//计时总时间,默认值设置为60
		std::thread m_thread;//计时运行于独立线程
		static int m_interval; //间隔时间，100ms
		std::atomic<bool>m_active{ false };//计时器是否已经启动
		//实现计时器的暂停与开启
		std::mutex m_mtx;//配合条件变量
		std::condition_variable m_cv;//条件用于阻塞唤醒线程
		std::atomic<bool>m_running{ false };//计时器是否正在运行，外部访问
		std::atomic<bool>m_pause{ false };//暂停标志：true=子线程应在下一轮停下等待
		std::atomic<bool>m_stop{ false };//退出循环的标志，即计时结束或取消
	};
}