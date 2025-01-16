#include "c.h"
#include <windows.h>
#include <stdio.h>

void drawVertical24hClock(BreezeState *state) {
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(state->bar.window, &ps);

	HBRUSH brush = CreateSolidBrush(state->bar.background);
	FillRect(dc, &(ps.rcPaint), brush);
	DeleteObject(brush);

	SetBkMode(dc, TRANSPARENT);
	SelectObject(dc, state->bar.current_font);
	SetTextColor(dc, state->bar.foreground);

	RECT bar_rect = ps.rcPaint;
	bar_rect.top += state->bar.padding;
	bar_rect.bottom -= state->bar.padding;

	RECT desktop_rect = bar_rect;
	desktop_rect.bottom = desktop_rect.top + state->bar.font_height;
	bar_rect.top = desktop_rect.bottom;
	char buf[16];
	snprintf(buf, len(buf)-1, "%zu", state->current_desktop);
	DrawTextA(dc, buf, -1, &desktop_rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);


	RECT minutes_rect = bar_rect;
	minutes_rect.top = minutes_rect.bottom - state->bar.font_height;
	bar_rect.bottom = minutes_rect.top; 

	RECT hours_rect = bar_rect;
	hours_rect.top = hours_rect.bottom - state->bar.font_height;
	bar_rect.bottom = hours_rect.top; 

	SYSTEMTIME lt;
	GetLocalTime(&lt);
	snprintf(buf, len(buf), "%02hu", lt.wHour);
	DrawTextA(dc, buf, -1, &hours_rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
	snprintf(buf, len(buf), "%02hu", lt.wMinute);
	DrawTextA(dc, buf, -1, &minutes_rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);

	EndPaint(state->bar.window, &ps);
}


void drawHorizontal24hClock(BreezeState *state) {
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(state->bar.window, &ps);

	HBRUSH brush = CreateSolidBrush(state->bar.background);
	FillRect(dc, &(ps.rcPaint), brush);
	DeleteObject(brush);

	SetBkMode(dc, TRANSPARENT);
	SelectObject(dc, state->bar.current_font);
	SetTextColor(dc, state->bar.foreground);

	RECT bar_rect = ps.rcPaint;
	bar_rect.left += state->bar.padding;
	bar_rect.right -= state->bar.padding;

	char buf[16];
	snprintf(buf, len(buf)-1, "%zu", state->current_desktop);
	DrawTextA(dc, buf, -1, &bar_rect, DT_VCENTER | DT_LEFT | DT_SINGLELINE);

	SYSTEMTIME lt;
	GetLocalTime(&lt);
	snprintf(buf, len(buf)-1, "%02hu:%02hu", lt.wHour, lt.wMinute);
	DrawTextA(dc, buf, -1, &bar_rect, DT_VCENTER | DT_RIGHT | DT_SINGLELINE);

	EndPaint(state->bar.window, &ps);
}
