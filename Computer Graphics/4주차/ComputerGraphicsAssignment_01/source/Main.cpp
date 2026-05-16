// Main.cpp
#include <windows.h>

#include "Shape.h"
#include "Rectangle.h"
#include "Circle.h"

bool g_drawRect = false;
bool g_drawCircle = false;
CShape* shapes[5] = { NULL };

LRESULT WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	shapes[0] = new CCircle(100, 100, 50);
	shapes[1] = new CRectangle(300, 300, 100, 100);
	shapes[2] = new CRectangle(200, 100, 50, 150);
	shapes[3] = new CCircle(100, 300, 150);
	shapes[4] = new CRectangle(200, 200, 200, 200);

	WNDCLASS wc;
	HWND hwnd;
	MSG msg;
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"MyWndClass";
	RegisterClass(&wc);
	hwnd = CreateWindow(
		L"MyWndClass", // WNDCLASS name
		L"SDK Application", // Window title
		WS_OVERLAPPEDWINDOW,// Window style
		CW_USEDEFAULT, // Horizontal position
		CW_USEDEFAULT, // Vertical position
		800, // Initial width
		800, // Initial height
		HWND_DESKTOP, // Handle of parent window
		NULL, // Menu handle
		hInstance, // Application's instance handle
		NULL // Window-creation data
	);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) {

	case WM_KEYDOWN:
		// press 'R' key
		if (wParam == 'R' || wParam == 'r') {
			g_drawRect = true;
			InvalidateRect(hwnd, NULL, TRUE);
		}
		return 0;

	case WM_RBUTTONDOWN:
		// press 'mouse right button'
		g_drawCircle = true;
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case WM_PAINT: // draw
		hdc = BeginPaint(hwnd, &ps);

		for (int i = 0; i < 5; i++) {
			// if draw rect
			if (g_drawRect) {
				CRectangle* rect = dynamic_cast<CRectangle*>(shapes[i]); 
				if (rect != NULL) {
					rect->Draw(hdc);
				}
			}

			// if draw circle 
			if (g_drawCircle) {
				CCircle* circle = dynamic_cast<CCircle*>(shapes[i]);
				if (circle != NULL) {
					circle->Draw(hdc);
				}
			}
		}

		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}