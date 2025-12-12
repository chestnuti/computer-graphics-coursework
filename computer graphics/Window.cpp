#include "./includes/Window.h"

Window* window;


void Window::create(int window_width, int window_height, std::string window_name) {
	// Register window class
	WNDCLASSEX wc;
	hinstance = GetModuleHandle(NULL);
	name = window_name;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	std::wstring wname = std::wstring(name.begin(), name.end());
	wc.lpszClassName = wname.c_str();
	wc.cbSize = sizeof(WNDCLASSEX);
	RegisterClassEx(&wc);
	// Create window
	width = window_width;
	height = window_height;
	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, wname.c_str(), wname.c_str(), style,
		0, 0, width, height, NULL, NULL, hinstance, this);

	window = this;
	// Hide cursor
	ShowCursor(FALSE);
}



void Window::processMessages() {
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static bool lockMouse = false;
	switch (msg)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		exit(0);
		return 0;
	}
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		exit(0);
		return 0;
	}
	case WM_KEYDOWN:
	{
		window->keys[(unsigned int)wParam] = true;
		return 0;
	}
	case WM_KEYUP:
	{
		window->keys[(unsigned int)wParam] = false;
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		window->mouseButtons[0] = true;
		return 0;
	}
	case WM_LBUTTONUP:
	{
		window->mouseButtons[0] = false;
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		window->mouseButtons[1] = true;
		return 0;
	}
	case WM_RBUTTONUP:
	{
		window->mouseButtons[1] = false;
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		//window->updateMouse(WINDOW_GET_X_LPARAM(lParam), WINDOW_GET_Y_LPARAM(lParam));
		if (!lockMouse) {
			int currentX = WINDOW_GET_X_LPARAM(lParam);
			int currentY = WINDOW_GET_Y_LPARAM(lParam);
			int centerX = window->width / 2;
			int centerY = window->height / 2;
			int deltaX = currentX - centerX;
			int deltaY = currentY - centerY;
			
			window->updateMouse(deltaX, deltaY);
			POINT centerPoint = { centerX, centerY };
			ClientToScreen(hwnd, &centerPoint);
			lockMouse = true;
			SetCursorPos(centerPoint.x, centerPoint.y);
			lockMouse = false;
		}
		return 0;
	}
	default:
	{
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	}
}
