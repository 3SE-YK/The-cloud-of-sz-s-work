#pragma once
#include <graphics.h>
#include <string>

namespace myTimer {
	class Timer;
}

namespace myPhoto {

	class Photo {
	public:
		Photo(int width = 800, int height = 600);
		~Photo();
		bool init();
		void show(int totalSeconds);
		void show(const myTimer::Timer& timer);
		void refresh();
		void close();
		bool isAlive() const;

		void clearCanvas();
		void drawButton(int x, int y, int w, int h, const std::wstring& text, bool hovered, bool pushed = false);
		void drawText(int x, int y, const std::wstring& text, int height = 24, int color = WHITE);
		void drawRect(int x, int y, int w, int h, int color, bool fill);
		void drawTimer(int totalSeconds, int x, int y, int textHeight = 64, int color = GREEN);
		bool getMouseClick(int& outX, int& outY);

		int width() const { return m_width; }
		int height() const { return m_height; }

	private:
		int m_width;
		int m_height;
		bool m_inited;
		void drawTime(int hours, int minutes, int seconds);
		std::wstring formatTime(int hours, int minutes, int seconds);
	};

}
