#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <atomic>
#include <map>

#include "windows.h"
#include "Common/Singleton.h"

namespace JoyEngine
{
	enum class KeyCode : uint8_t
	{
		KEYCODE_ENTER = VK_RETURN,
		KEYCODE_LSHIFT = VK_LSHIFT,
		KEYCODE_RSHIFT = VK_RSHIFT,
		KEYCODE_ESCAPE = VK_ESCAPE,
		KEYCODE_TAB = VK_TAB,
		KEYCODE_LALT = VK_LMENU,
		KEYCODE_RALT = VK_RMENU,
		KEYCODE_A = 'A',
		KEYCODE_B = 'B',
		KEYCODE_C = 'C',
		KEYCODE_D = 'D',
		KEYCODE_E = 'E',
		KEYCODE_F = 'F',
		KEYCODE_G = 'G',
		KEYCODE_H = 'H',
		KEYCODE_I = 'I',
		KEYCODE_J = 'J',
		KEYCODE_K = 'K',
		KEYCODE_L = 'L',
		KEYCODE_M = 'M',
		KEYCODE_N = 'N',
		KEYCODE_O = 'O',
		KEYCODE_P = 'P',
		KEYCODE_Q = 'Q',
		KEYCODE_R = 'R',
		KEYCODE_S = 'S',
		KEYCODE_T = 'T',
		KEYCODE_U = 'U',
		KEYCODE_V = 'V',
		KEYCODE_W = 'W',
		KEYCODE_X = 'X',
		KEYCODE_Y = 'Y',
		KEYCODE_Z = 'Z',
	};

	class InputManager : public Singleton<InputManager>
	{
	public:
		InputManager();
		void HandleWinMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		bool GetKeyDown(KeyCode code) const;
		bool GetKeyUp(KeyCode code) const;

	private:
		std::atomic<bool> m_keyStates[1 << (sizeof(KeyCode) * 8)];
	};
}

#endif // INPUT_MANAGER_H
