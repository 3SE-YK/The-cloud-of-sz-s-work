#include "Photo.h"
#include <sstream>
#include <iomanip>
#include "Timer.h"

using namespace myPhoto;

Photo::Photo(int width, int height)
	: m_width(width), m_height(height), m_inited(false) {
}

Photo::~Photo() {
	if (m_inited) close();
}

bool Photo::init() {
	m_inited = (initgraph(m_width, m_height) != NULL);
	if (m_inited) {
		setbkcolor(BLACK);
		cleardevice();
		BeginBatchDraw();
	}
	return m_inited;
}

void Photo::show(int totalSeconds) {
	if (!m_inited) return;
	int hours = totalSeconds / 3600;
	int minutes = (totalSeconds / 60) % 60;
	int seconds = totalSeconds % 60;
	clearCanvas();
	drawTime(hours, minutes, seconds);
	refresh();
}

void Photo::show(const myTimer::Timer& timer) {
	show(timer.getPeriod());
}

void Photo::clearCanvas() {
	if (!m_inited) return;
	cleardevice();
}

void Photo::drawTime(int hours, int minutes, int seconds) {
	settextstyle(64, 0, _T("Consolas"));
	setcolor(GREEN);
	setbkmode(TRANSPARENT);
	std::wstring timeStr = formatTime(hours, minutes, seconds);
	int tw = textwidth(timeStr.c_str());
	int th = textheight(timeStr.c_str());
	int x = (m_width - tw) / 2;
	int y = (m_height - th) / 2;
	outtextxy(x, y, timeStr.c_str());
}

std::wstring Photo::formatTime(int hours, int minutes, int seconds) {
	std::wstringstream ss;
	ss << std::setfill(L'0') << std::setw(2) << hours << L":"
		<< std::setw(2) << minutes << L":"
		<< std::setw(2) << seconds;
	return ss.str();
}

void Photo::drawTimer(int totalSeconds, int x, int y, int textHeight, int color) {
	if (!m_inited) return;
	int hours = totalSeconds / 3600;
	int minutes = (totalSeconds / 60) % 60;
	int seconds = totalSeconds % 60;
	std::wstring timeStr = formatTime(hours, minutes, seconds);
	settextstyle(textHeight, 0, _T("Consolas"));
	setcolor(color);
	setbkmode(TRANSPARENT);
	int tw = textwidth(timeStr.c_str());
	int th = textheight(timeStr.c_str());
	outtextxy(x - tw / 2, y - th / 2, timeStr.c_str());
}

void Photo::refresh() {
	if (m_inited) FlushBatchDraw();
}

void Photo::close() {
	if (m_inited) {
		EndBatchDraw();
		closegraph();
		m_inited = false;
	}
}

bool Photo::isAlive() const {
	return m_inited && IsWindow(GetHWnd());
}

void Photo::drawRect(int x, int y, int w, int h, int color, bool fill) {
	if (!m_inited) return;
	setlinecolor(color);
	if (fill) {
		setfillcolor(color);
		fillrectangle(x, y, x + w, y + h);
	}
	else {
		rectangle(x, y, x + w, y + h);
	}
}

void Photo::drawButton(int x, int y, int w, int h, const std::wstring& text, bool hovered, bool pushed) {
	if (!m_inited) return;
	// ¹Ø¼üÐÞ¸´£ºEasyX Ã»ÓÐ GRAY ºê£¬ÓÃ RGB(128,128,128) ´úÌæ
	int fillColor = hovered ? (pushed ? DARKGRAY : RGB(128, 128, 128)) : LIGHTGRAY;
	int textColor = BLACK;
	setfillcolor(fillColor);
	setlinecolor(WHITE);
	fillrectangle(x, y, x + w, y + h);
	rectangle(x, y, x + w, y + h);

	setbkmode(TRANSPARENT);
	settextstyle(24, 0, _T("Î¢ÈíÑÅºÚ"));
	setcolor(textColor);
	int tw = textwidth(text.c_str());
	int th = textheight(text.c_str());
	outtextxy(x + (w - tw) / 2, y + (h - th) / 2, text.c_str());
}

void Photo::drawText(int x, int y, const std::wstring& text, int height, int color) {
	if (!m_inited) return;
	setbkmode(TRANSPARENT);
	settextstyle(height, 0, _T("Î¢ÈíÑÅºÚ"));
	setcolor(color);
	int tw = textwidth(text.c_str());
	int th = textheight(text.c_str());
	outtextxy(x - tw / 2, y - th / 2, text.c_str());
}

bool Photo::getMouseClick(int& outX, int& outY) {
	if (!m_inited) return false;
	ExMessage msg;
	if (peekmessage(&msg, EX_MOUSE)) {
		if (msg.message == WM_LBUTTONDOWN) {
			outX = msg.x;
			outY = msg.y;
			return true;
		}
	}
	return false;
}
