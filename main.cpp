#include "Timer.h"
#include "Photo.h"
#include "Task.h"
#include <thread>
#include <chrono>
#include <memory>
#include <string>
#include <windows.h>
#include <graphics.h>

using namespace myTimer;
using namespace myPhoto;
using namespace myTask;

enum class AppState {
	MAIN_MENU,
	CREATE_TASK,
	RUNNING_TASK,
	TASK_SUMMARY
};

AppState g_state = AppState::MAIN_MENU;
std::unique_ptr<Task> g_task;
std::string g_inputName;
std::string g_inputDetail;
int g_inputHour = 0;
int g_inputMinute = 0;
int g_inputSecond = 0;
bool g_isPaused = false;
std::wstring g_summaryMsg;

bool isIn(int mx, int my, int x, int y, int w, int h) {
	return mx >= x && mx < x + w && my >= y && my < y + h;
}

std::wstring to_wstring(const std::string& str) {
	if (str.empty()) return std::wstring();
	int size = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	if (size <= 0) return std::wstring();
	std::wstring wstr(size, 0);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr[0], size);
	wstr.pop_back();
	return wstr;
}

bool inputString(const wchar_t* prompt, std::string& out) {
	wchar_t buf[512] = { 0 };
	if (InputBox((LPTSTR)buf, 512, prompt, _T("输入"), NULL) > 0) {
		int len = WideCharToMultiByte(CP_ACP, 0, buf, -1, NULL, 0, NULL, NULL);
		if (len > 0) {
			std::string str(len, 0);
			WideCharToMultiByte(CP_ACP, 0, buf, -1, &str[0], len, NULL, NULL);
			str.pop_back();
			out = str;
			return true;
		}
	}
	return false;
}

bool inputNumber(const wchar_t* prompt, int& out) {
	wchar_t buf[64] = { 0 };
	if (InputBox((LPTSTR)buf, 64, prompt, _T("输入"), NULL, 0, 0, true) > 0) {
		out = (int)_wtoi(buf);
		return true;
	}
	return false;
}

void renderMainMenu(Photo& photo) {
	photo.clearCanvas();
	photo.drawText(photo.width() / 2, 100, L"日程表", 48, WHITE);
	photo.drawText(photo.width() / 2, 180, L"高效管理您的每一个任务", 20, LIGHTGRAY);
	photo.drawButton(250, 280, 300, 60, L"创建新任务", false);
	if (g_task) {
		photo.drawText(photo.width() / 2, 400, L"提示：已有任务正在进行或刚结束", 18, YELLOW);
	}
	else {
		photo.drawText(photo.width() / 2, 400, L"暂无任务，点击上方按钮开始", 18, DARKGRAY);
	}
	photo.refresh();
}

void renderCreateTask(Photo& photo) {
	photo.clearCanvas();
	photo.drawText(photo.width() / 2, 50, L"创建新任务", 36, WHITE);

	photo.drawText(130, 120, L"任务名称", 20, WHITE);
	photo.drawRect(250, 115, 400, 30, DARKGRAY, true);
	std::wstring nameText = g_inputName.empty() ? std::wstring(L"(点击设置名称)") : to_wstring(g_inputName);
	// 关键修复：GRAY 改为 RGB(128,128,128)
	photo.drawText(450, 132, nameText, 18, g_inputName.empty() ? RGB(128, 128, 128) : WHITE);

	photo.drawText(130, 180, L"预计时间", 20, WHITE);
	photo.drawRect(250, 175, 400, 30, DARKGRAY, true);
	std::wstring timeStr = std::to_wstring(g_inputHour) + L" 时 "
		+ std::to_wstring(g_inputMinute) + L" 分 "
		+ std::to_wstring(g_inputSecond) + L" 秒";
	photo.drawText(450, 192, timeStr, 18, WHITE);

	photo.drawText(130, 240, L"任务内容", 20, WHITE);
	photo.drawRect(250, 235, 400, 100, DARKGRAY, true);
	std::wstring detailText = g_inputDetail.empty() ? std::wstring(L"(点击设置内容)") : to_wstring(g_inputDetail);
	if (detailText.length() > 30) detailText = detailText.substr(0, 30) + L"...";
	// 关键修复：GRAY 改为 RGB(128,128,128)
	photo.drawText(450, 285, detailText, 18, g_inputDetail.empty() ? RGB(128, 128, 128) : WHITE);

	photo.drawButton(150, 380, 160, 45, L"设置名称", false);
	photo.drawButton(340, 380, 160, 45, L"设置时间", false);
	photo.drawButton(530, 380, 160, 45, L"设置内容", false);
	photo.drawButton(200, 460, 160, 45, L"确认创建", false);
	photo.drawButton(440, 460, 160, 45, L"返回", false);

	photo.refresh();
}

void renderRunningTask(Photo& photo) {
	photo.clearCanvas();
	if (!g_task) return;

	photo.drawText(photo.width() / 2, 60, to_wstring(g_task->m_name), 32, YELLOW);

	int rem = g_task->getRemaining();
	if (rem < 0) rem = 0;
	photo.drawTimer(rem, photo.width() / 2, 200, 72, GREEN);

	std::wstring detail = to_wstring(g_task->m_detail);
	if (detail.length() > 40) detail = detail.substr(0, 40) + L"...";
	photo.drawText(photo.width() / 2, 320, detail, 20, WHITE);

	if (g_isPaused) {
		photo.drawText(photo.width() / 2, 370, L"状态: 已暂停", 20, RED);
	}
	else {
		photo.drawText(photo.width() / 2, 370, L"状态: 进行中", 20, GREEN);
	}

	std::wstring pauseText = g_isPaused ? L"继续" : L"暂停";
	photo.drawButton(200, 430, 150, 50, pauseText, false);
	photo.drawButton(450, 430, 150, 50, L"结束任务", false);

	photo.refresh();
}

void renderSummary(Photo& photo) {
	photo.clearCanvas();
	photo.drawText(photo.width() / 2, 200, g_summaryMsg, 40, WHITE);
	photo.drawButton(300, 350, 200, 50, L"返回主菜单", false);
	photo.refresh();
}

void updateMainMenu(Photo& photo, bool clicked, int mx, int my) {
	if (clicked && isIn(mx, my, 250, 280, 300, 60)) {
		g_state = AppState::CREATE_TASK;
		g_inputName.clear();
		g_inputDetail.clear();
		g_inputHour = g_inputMinute = g_inputSecond = 0;
	}
	renderMainMenu(photo);
}

void updateCreateTask(Photo& photo, bool clicked, int mx, int my) {
	if (clicked) {
		if (isIn(mx, my, 150, 380, 160, 45)) {
			inputString(L"请输入任务名称", g_inputName);
		}
		else if (isIn(mx, my, 340, 380, 160, 45)) {
			int h = 0, m = 0, s = 0;
			if (inputNumber(L"请输入小时 (0-99)", h)) g_inputHour = h;
			if (inputNumber(L"请输入分钟 (0-59)", m)) g_inputMinute = m;
			if (inputNumber(L"请输入秒钟 (0-59)", s)) g_inputSecond = s;
		}
		else if (isIn(mx, my, 530, 380, 160, 45)) {
			inputString(L"请输入任务具体内容", g_inputDetail);
		}
		else if (isIn(mx, my, 200, 460, 160, 45)) {
			if (g_inputName.empty()) {
				MessageBox(GetHWnd(), _T("任务名称不能为空"), _T("提示"), MB_OK);
			}
			else {
				int totalSec = g_inputHour * 3600 + g_inputMinute * 60 + g_inputSecond;
				if (totalSec <= 0) {
					MessageBox(GetHWnd(), _T("预计时间必须大于0"), _T("提示"), MB_OK);
				}
				else {
					g_task = std::make_unique<Task>(g_inputName, g_inputDetail, g_inputHour, g_inputMinute, g_inputSecond);
					g_isPaused = false;
					g_task->start();
					g_state = AppState::RUNNING_TASK;
				}
			}
		}
		else if (isIn(mx, my, 440, 460, 160, 45)) {
			g_state = AppState::MAIN_MENU;
		}
	}
	renderCreateTask(photo);
}

void updateRunningTask(Photo& photo, bool clicked, int mx, int my) {
	if (!g_task) {
		g_state = AppState::MAIN_MENU;
		return;
	}

	if (clicked) {
		if (isIn(mx, my, 200, 430, 150, 50)) {
			if (g_isPaused) {
				g_task->resume();
				g_isPaused = false;
			}
			else {
				g_task->pause();
				g_isPaused = true;
			}
		}
		else if (isIn(mx, my, 450, 430, 150, 50)) {
			g_task->stop();
			g_task->wait();
			g_summaryMsg = L"任务已手动结束";
			g_task.reset();
			g_state = AppState::TASK_SUMMARY;
			return;
		}
	}

	if (g_task->isFinished()) {
		g_task->wait();
		g_summaryMsg = L"任务完成！";
		g_task.reset();
		g_state = AppState::TASK_SUMMARY;
		return;
	}

	renderRunningTask(photo);
}

void updateSummary(Photo& photo, bool clicked, int mx, int my) {
	if (clicked && isIn(mx, my, 300, 350, 200, 50)) {
		g_state = AppState::MAIN_MENU;
	}
	renderSummary(photo);
}

int main() {
	Photo photo(800, 600);
	if (!photo.init()) {
		return -1;
	}

	while (photo.isAlive()) {
		int mx = -1, my = -1;
		bool clicked = photo.getMouseClick(mx, my);

		switch (g_state) {
		case AppState::MAIN_MENU:
			updateMainMenu(photo, clicked, mx, my);
			break;
		case AppState::CREATE_TASK:
			updateCreateTask(photo, clicked, mx, my);
			break;
		case AppState::RUNNING_TASK:
			updateRunningTask(photo, clicked, mx, my);
			break;
		case AppState::TASK_SUMMARY:
			updateSummary(photo, clicked, mx, my);
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}

	if (g_task) {
		g_task->stop();
		g_task->wait();
	}
	photo.close();
	return 0;
}
