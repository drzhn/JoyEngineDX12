#ifndef WINDOW_HANDLER_H
#define WINDOW_HANDLER_H

#include "windows.h"
#include "JoyEngine.h"

class WindowHandler
{
public :
	static void RegisterMessageHandler(JoyEngine::IWindowHandler* messageHandler, HWND hwnd);

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static bool GetWindowDestroyed() { return m_windowDestroyed; }

private :
	static bool m_windowDestroyed;
	static JoyEngine::IWindowHandler* m_messageHandler;
	static HWND m_hwnd;
};

#endif // WINDOW_HANDLER_H
