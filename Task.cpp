#include "Task.h"
#include <iostream>
using namespace myTask;

Task::Task() : m_name("未命名"), m_detail("无"), m_totalSeconds(0), m_timer(0) {}

Task::Task(std::string name, std::string detail, int hour, int minute, int second)
	: m_name(name), m_detail(detail),
	m_totalSeconds(hour * 3600 + minute * 60 + second),
	m_timer(m_totalSeconds)
{
	writeLog();
}

Task::~Task() {
	if (m_started.load() && m_timerThread.joinable()) {
		m_timer.stop();
		m_timerThread.join();
	}
}

void Task::start() {
	if (m_started.load()) return;
	m_started.store(true);
	m_timerThread = std::thread([this]() {
		m_timer.start();
		});
}

void Task::stop() {
	m_timer.stop();
}

void Task::pause() {
	m_timer.kick_off();
}

void Task::resume() {
	m_timer.kick_on();
}

void Task::wait() {
	if (m_timerThread.joinable()) {
		m_timerThread.join();
	}
}

int Task::getRemaining() const {
	return m_timer.getPeriod();
}

bool Task::isFinished() const {
	return m_timer.getPeriod() <= 0;
}

void Task::writeLog() const {
	std::string file_name = "Task_" + m_name + ".txt";
	std::ofstream ofs(file_name, std::ios::out);
	if (!ofs) {
		std::cerr << "无法创建日志文件: " << file_name << "\n";
		return;
	}
	ofs << "任务名称: " << m_name << "\n";
	ofs << "预计时长: " << m_totalSeconds / 3600 << "小时 "
		<< (m_totalSeconds / 60) % 60 << "分钟 "
		<< m_totalSeconds % 60 << "秒\n";
	ofs << "任务内容: " << m_detail << "\n";
	ofs.close();
}
