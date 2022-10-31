#include "WindowHandler.h"

#include <JoyEngine.h>
#include <string>

HWND WindowHandler::m_hwnd = nullptr;
bool WindowHandler::m_windowDestroyed = false;
JoyEngine::IWindowHandler* WindowHandler::m_messageHandler = nullptr;

void WindowHandler::RegisterMessageHandler(JoyEngine::IWindowHandler* messageHandler, HWND hwnd)
{
	m_messageHandler = messageHandler;
	m_messageHandler->SetDeltaTimeHandler([hwnd](float deltaTime)
	{
		SetWindowTextA(hwnd, (
#ifdef _DEBUG
			
#ifdef FULL_DEBUG
			               "JoyEngine FULL DEBUG   " +
#else
			               "JoyEngine DEBUG   " +
#endif

#else
						   "JoyEngine RELEASE   " +
#endif
			               std::to_string(static_cast<int>(1 / deltaTime)) +
			               " FPS"
		               ).c_str());
	});
	m_hwnd = hwnd;
}

LRESULT WindowHandler::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		break;
	case WM_DESTROY:
		{
			m_windowDestroyed = true;
			PostQuitMessage(0);
			break;
		}
	default:
		{
			if (m_messageHandler != nullptr && hwnd == m_hwnd)
			{
				m_messageHandler->HandleMessage(hwnd, uMsg, wParam, lParam);
			}
			break;
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
