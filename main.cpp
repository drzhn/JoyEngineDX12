#ifndef UNICODE
#define UNICODE
#endif


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#include "WindowHandler.h"
#include "JoyContext.h"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <memory>


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	constexpr uint32_t windowWidth = 1280;
	constexpr uint32_t windowHeight = 720;

	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Sample Window Class";
	WNDCLASS wc = {};
	wc.lpfnWndProc = &WindowHandler::WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	RECT wr = {0, 0, windowWidth, windowHeight};
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hwnd = CreateWindowEx(
		0, // Optional window styles.
		CLASS_NAME, // Window class
		L"", // Window text
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // Window style
		// Size and position
		100, 100,
		wr.right - wr.left, // width
		wr.bottom - wr.top,
		nullptr, // Parent window
		nullptr, // Menu
		hInstance, // Instance handle
		nullptr // Additional application data
	);

	//HWND hwndButton = CreateWindow(
	//	L"STATIC", // Predefined class; Unicode assumed 
	//	L"OK", // Button text 
	//	WS_VISIBLE | WS_CHILD, // | BS_DEFPUSHBUTTON, // Styles 
	//	10, // x position 
	//	10, // y position 
	//	100, // Button width
	//	100, // Button height
	//	hwnd, // Parent window
	//	NULL, // No menu.
	//	(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	//	NULL); // Pointer not needed.

	//SetWindowTextA(hwndButton, "sdfsdfsdfsdfsdfsdfsdfsdfsdfsdf");
	if (hwnd == nullptr)
	{
		return 0;
	}

	JoyEngine::JoyEngine* graphicsContext = new JoyEngine::JoyEngine(hInstance, hwnd, windowWidth, windowHeight);
	WindowHandler::RegisterMessageHandler(graphicsContext, hwnd);

	graphicsContext->Init();


	ShowWindow(hwnd, nCmdShow);

	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (WindowHandler::GetWindowDestroyed())
		{
			break;
		}
		graphicsContext->Update();
	}

	delete graphicsContext;

	return 0;
}
