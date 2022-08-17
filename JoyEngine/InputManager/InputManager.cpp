#include "InputManager.h"

namespace JoyEngine
{
	IMPLEMENT_SINGLETON(InputManager)

	void InputManager::HandleWinMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_KEYDOWN)
		{
			switch (wParam)
			{
			case VK_RETURN: m_keyStates[KeyCode::KEYCODE_ENTER] = true;
				break;
			case VK_LSHIFT: m_keyStates[KeyCode::KEYCODE_LSHIFT] = true;
				break;
			case VK_RSHIFT: m_keyStates[KeyCode::KEYCODE_RSHIFT] = true;
				break;
			case VK_ESCAPE: m_keyStates[KeyCode::KEYCODE_ESCAPE] = true;
				break;
			case 'Q': m_keyStates[KeyCode::KEYCODE_Q] = true;
				break;
			case 'W': m_keyStates[KeyCode::KEYCODE_W] = true;
				break;
			case 'E': m_keyStates[KeyCode::KEYCODE_E] = true;
				break;
			case 'R': m_keyStates[KeyCode::KEYCODE_R] = true;
				break;
			case 'T': m_keyStates[KeyCode::KEYCODE_T] = true;
				break;
			case 'Y': m_keyStates[KeyCode::KEYCODE_Y] = true;
				break;
			case 'U': m_keyStates[KeyCode::KEYCODE_U] = true;
				break;
			case 'I': m_keyStates[KeyCode::KEYCODE_I] = true;
				break;
			case 'O': m_keyStates[KeyCode::KEYCODE_O] = true;
				break;
			case 'P': m_keyStates[KeyCode::KEYCODE_P] = true;
				break;
			case 'A': m_keyStates[KeyCode::KEYCODE_A] = true;
				break;
			case 'S': m_keyStates[KeyCode::KEYCODE_S] = true;
				break;
			case 'D': m_keyStates[KeyCode::KEYCODE_D] = true;
				break;
			case 'F': m_keyStates[KeyCode::KEYCODE_F] = true;
				break;
			case 'G': m_keyStates[KeyCode::KEYCODE_G] = true;
				break;
			case 'H': m_keyStates[KeyCode::KEYCODE_H] = true;
				break;
			case 'J': m_keyStates[KeyCode::KEYCODE_J] = true;
				break;
			case 'K': m_keyStates[KeyCode::KEYCODE_K] = true;
				break;
			case 'L': m_keyStates[KeyCode::KEYCODE_L] = true;
				break;
			case 'Z': m_keyStates[KeyCode::KEYCODE_Z] = true;
				break;
			case 'X': m_keyStates[KeyCode::KEYCODE_X] = true;
				break;
			case 'C': m_keyStates[KeyCode::KEYCODE_C] = true;
				break;
			case 'V': m_keyStates[KeyCode::KEYCODE_V] = true;
				break;
			case 'B': m_keyStates[KeyCode::KEYCODE_B] = true;
				break;
			case 'N': m_keyStates[KeyCode::KEYCODE_N] = true;
				break;
			case 'M': m_keyStates[KeyCode::KEYCODE_M] = true;
				break;
			default: ;
			}
		}
		if (uMsg == WM_KEYUP)
		{
			switch (wParam)
			{
			case VK_RETURN: m_keyStates[KeyCode::KEYCODE_ENTER] = false;
				break;
			case VK_LSHIFT: m_keyStates[KeyCode::KEYCODE_LSHIFT] = false;
				break;
			case VK_RSHIFT: m_keyStates[KeyCode::KEYCODE_RSHIFT] = false;
				break;
			case VK_ESCAPE: m_keyStates[KeyCode::KEYCODE_ESCAPE] = false;
				break;
			case 'Q': m_keyStates[KeyCode::KEYCODE_Q] = false;
				break;
			case 'W': m_keyStates[KeyCode::KEYCODE_W] = false;
				break;
			case 'E': m_keyStates[KeyCode::KEYCODE_E] = false;
				break;
			case 'R': m_keyStates[KeyCode::KEYCODE_R] = false;
				break;
			case 'T': m_keyStates[KeyCode::KEYCODE_T] = false;
				break;
			case 'Y': m_keyStates[KeyCode::KEYCODE_Y] = false;
				break;
			case 'U': m_keyStates[KeyCode::KEYCODE_U] = false;
				break;
			case 'I': m_keyStates[KeyCode::KEYCODE_I] = false;
				break;
			case 'O': m_keyStates[KeyCode::KEYCODE_O] = false;
				break;
			case 'P': m_keyStates[KeyCode::KEYCODE_P] = false;
				break;
			case 'A': m_keyStates[KeyCode::KEYCODE_A] = false;
				break;
			case 'S': m_keyStates[KeyCode::KEYCODE_S] = false;
				break;
			case 'D': m_keyStates[KeyCode::KEYCODE_D] = false;
				break;
			case 'F': m_keyStates[KeyCode::KEYCODE_F] = false;
				break;
			case 'G': m_keyStates[KeyCode::KEYCODE_G] = false;
				break;
			case 'H': m_keyStates[KeyCode::KEYCODE_H] = false;
				break;
			case 'J': m_keyStates[KeyCode::KEYCODE_J] = false;
				break;
			case 'K': m_keyStates[KeyCode::KEYCODE_K] = false;
				break;
			case 'L': m_keyStates[KeyCode::KEYCODE_L] = false;
				break;
			case 'Z': m_keyStates[KeyCode::KEYCODE_Z] = false;
				break;
			case 'X': m_keyStates[KeyCode::KEYCODE_X] = false;
				break;
			case 'C': m_keyStates[KeyCode::KEYCODE_C] = false;
				break;
			case 'V': m_keyStates[KeyCode::KEYCODE_V] = false;
				break;
			case 'B': m_keyStates[KeyCode::KEYCODE_B] = false;
				break;
			case 'N': m_keyStates[KeyCode::KEYCODE_N] = false;
				break;
			case 'M': m_keyStates[KeyCode::KEYCODE_M] = false;
				break;
			default:;
			}
		}
	}

	bool InputManager::GetKeyDown(KeyCode code)
	{
		return m_keyStates[code] == true;
	}

	bool InputManager::GetKeyUp(KeyCode code)
	{
		return m_keyStates[code] == false;
	}
}
