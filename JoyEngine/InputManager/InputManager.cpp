#include "InputManager.h"

#include "Utils/Assert.h"

namespace JoyEngine
{
	InputManager::InputManager()
	{
		m_keyStates.emplace(KeyCode::KEYCODE_ENTER, false);
		m_keyStates.emplace(KeyCode::KEYCODE_LSHIFT, false);
		m_keyStates.emplace(KeyCode::KEYCODE_RSHIFT, false);
		m_keyStates.emplace(KeyCode::KEYCODE_ESCAPE, false);
		m_keyStates.emplace(KeyCode::KEYCODE_TAB, false);
		m_keyStates.emplace(KeyCode::KEYCODE_LALT, false);
		m_keyStates.emplace(KeyCode::KEYCODE_RALT, false);
		m_keyStates.emplace(KeyCode::KEYCODE_A, false);
		m_keyStates.emplace(KeyCode::KEYCODE_B, false);
		m_keyStates.emplace(KeyCode::KEYCODE_C, false);
		m_keyStates.emplace(KeyCode::KEYCODE_D, false);
		m_keyStates.emplace(KeyCode::KEYCODE_E, false);
		m_keyStates.emplace(KeyCode::KEYCODE_F, false);
		m_keyStates.emplace(KeyCode::KEYCODE_G, false);
		m_keyStates.emplace(KeyCode::KEYCODE_H, false);
		m_keyStates.emplace(KeyCode::KEYCODE_I, false);
		m_keyStates.emplace(KeyCode::KEYCODE_J, false);
		m_keyStates.emplace(KeyCode::KEYCODE_K, false);
		m_keyStates.emplace(KeyCode::KEYCODE_L, false);
		m_keyStates.emplace(KeyCode::KEYCODE_M, false);
		m_keyStates.emplace(KeyCode::KEYCODE_N, false);
		m_keyStates.emplace(KeyCode::KEYCODE_O, false);
		m_keyStates.emplace(KeyCode::KEYCODE_P, false);
		m_keyStates.emplace(KeyCode::KEYCODE_Q, false);
		m_keyStates.emplace(KeyCode::KEYCODE_R, false);
		m_keyStates.emplace(KeyCode::KEYCODE_S, false);
		m_keyStates.emplace(KeyCode::KEYCODE_T, false);
		m_keyStates.emplace(KeyCode::KEYCODE_U, false);
		m_keyStates.emplace(KeyCode::KEYCODE_V, false);
		m_keyStates.emplace(KeyCode::KEYCODE_W, false);
		m_keyStates.emplace(KeyCode::KEYCODE_X, false);
		m_keyStates.emplace(KeyCode::KEYCODE_Y, false);
		m_keyStates.emplace(KeyCode::KEYCODE_Z, false);
	}

	void InputManager::HandleWinMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_KEYDOWN)
		{
			m_keyStates[static_cast<KeyCode>(wParam)].store(true, std::memory_order::release);
		}
		if (uMsg == WM_KEYUP)
		{
			m_keyStates[static_cast<KeyCode>(wParam)].store(false, std::memory_order::release);
		}
	}

	bool InputManager::GetKeyDown(KeyCode code) const
	{
		if (!m_keyStates.contains(code))
		{
			ASSERT(false)
		}
		return m_keyStates.at(code).load(std::memory_order::acquire) == true;
	}

	bool InputManager::GetKeyUp(KeyCode code) const
	{
		if (!m_keyStates.contains(code))
		{
			ASSERT(false)
		}
		return m_keyStates.at(code).load(std::memory_order::acquire) == false;
	}
}
