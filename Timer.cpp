#include"Timer.h"
using namespace myTimer;
int Timer::m_interval = 1000;
Timer::Timer(int period ) :
	m_running{ false }, m_period{ period }
{
	;
}
Timer::~Timer() {};
void Timer::start()
{
	//修改计时器状态
	m_active.store(true);//计时器已经启动
	m_pause.store(false);//未暂停，正在计时
	m_stop.store(false);
	m_thread = std::thread([this]() {
		m_running.store(true);//线程正在进行
		int accumulated = 0; // 本轮已经累积的时间
		while (m_period.load() > 0 && !m_stop.load())
		{
			//如果处于暂停状态（pause=true），阻塞等待，立即响应kick_on
			std::unique_lock<std::mutex> lock(m_mtx);
			m_cv.wait(lock, [this]() {return !m_pause.load() || m_stop.load(); });
			//工作逻辑：唤醒，拿到锁，检查m_pause 为false时执行lock，当前子线程拿到锁进行计时，否则继续休眠并释放锁  
			//当休眠结束是因为stop，直接退出循环
			if (m_stop.load()) break;
			//退出wait后立即释放锁，否则主线程的开关会卡死
			lock.unlock();

			//进入休眠计时,100ms
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			accumulated += 100;
			//休眠期间主线程调用kick off暂停，休眠结束立马响应
			if (m_pause.load()) continue;
			if (accumulated >= m_interval) {
				accumulated -= 1000;
				//达到100ms，需修改m_period
				if (m_period.load() > 0) m_period--;
			}
		}
		//循环结束，输出信息
		std::cout << "计时已经结束" << '\n';
		m_running.store(false);
		});
	join_thread();
}
void Timer::kick_off()
{
	std::lock_guard<std::mutex>lock(m_mtx);//对主线程进行加锁保护，确保主线程在修改状态时不会意外进入子线程
	m_pause.store(true);
	m_cv.notify_one();
}
void Timer::kick_on()
{
	std::lock_guard<std::mutex>lock(m_mtx);//对主线程进行加锁保护，确保主线程在修改状态时不会意外进入子线程
	m_pause.store(false);
	m_cv.notify_one();
}

void Timer::stop()
{
	std::lock_guard<std::mutex> lock(m_mtx);
	m_stop.store(true);
	m_cv.notify_one();
}

void Timer::reset(int period)
{
	m_period.store(period);
	m_stop.store(false);
	m_pause.store(false);
	m_active.store(false);
	m_running.store(false);
}
