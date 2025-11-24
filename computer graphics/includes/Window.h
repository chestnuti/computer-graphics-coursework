#pragma once
#include <Windows.h>
#include <iostream>
#include <string>

#define WINDOW_GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define WINDOW_GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


class Window {
public:
	HWND hwnd;
	HINSTANCE hinstance;
	std::string name;
	int width;
	int height;

	bool keys[256] = { false };
	bool mouseButtons[3] = { false };
	int mousex;
	int mousey;

	Window() {}
	void create(int width, int height, std::string window_name);
	void updateMouse(int x, int y)
	{
		mousex = x;
		mousey = y;
	}
	void processMessages();


};
