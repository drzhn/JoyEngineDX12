#include "InputManager.h"

#include "Utils/Assert.h"

namespace JoyEngine
{
	InputManager::InputManager()
	{
		for (uint32_t i = 0; i < sizeof(KeyCode) * 8; i++)
		{
			m_keyStates[i].store(false, std::memory_order::release);
		}
	}

	void InputManager::HandleWinMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_KEYDOWN)
		{
			m_keyStates[wParam].store(true, std::memory_order::release);
		}
		if (uMsg == WM_KEYUP)
		{
			m_keyStates[wParam].store(false, std::memory_order::release);
		}
	}

	bool InputManager::GetKeyDown(KeyCode code) const
	{
		return m_keyStates[static_cast<uint8_t>(code)].load(std::memory_order::acquire) == true;
	}

	bool InputManager::GetKeyUp(KeyCode code) const
	{
		return m_keyStates[static_cast<uint8_t>(code)].load(std::memory_order::acquire) == false;
	}
}
