#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <map>

#include "windows.h"
#include "Common/Singleton.h"

namespace JoyEngine
{
	enum class KeyCode
	{
		KEYCODE_Q,
		KEYCODE_W,
		KEYCODE_E,
		KEYCODE_R,
		KEYCODE_T,
		KEYCODE_Y,
		KEYCODE_U,
		KEYCODE_I,
		KEYCODE_O,
		KEYCODE_P,
		KEYCODE_A,
		KEYCODE_S,
		KEYCODE_D,
		KEYCODE_F,
		KEYCODE_G,
		KEYCODE_H,
		KEYCODE_J,
		KEYCODE_K,
		KEYCODE_L,
		KEYCODE_Z,
		KEYCODE_X,
		KEYCODE_C,
		KEYCODE_V,
		KEYCODE_B,
		KEYCODE_N,
		KEYCODE_M,
		KEYCODE_TAB,
		KEYCODE_ENTER,
		KEYCODE_LSHIFT,
		KEYCODE_RSHIFT,
		KEYCODE_LALT,
		KEYCODE_RALT,
		KEYCODE_ESCAPE,
	};

	class InputManager : public Singleton<InputManager>
	{
	public:
		InputManager() = default;
		void HandleWinMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		bool GetKeyDown(KeyCode code);
		bool GetKeyUp(KeyCode code);
	private:
		std::map<KeyCode, bool> m_keyStates{
			{KeyCode::KEYCODE_Q, false},
			{KeyCode::KEYCODE_W, false},
			{KeyCode::KEYCODE_E, false},
			{KeyCode::KEYCODE_R, false},
			{KeyCode::KEYCODE_T, false},
			{KeyCode::KEYCODE_Y, false},
			{KeyCode::KEYCODE_U, false},
			{KeyCode::KEYCODE_I, false},
			{KeyCode::KEYCODE_O, false},
			{KeyCode::KEYCODE_P, false},
			{KeyCode::KEYCODE_A, false},
			{KeyCode::KEYCODE_S, false},
			{KeyCode::KEYCODE_D, false},
			{KeyCode::KEYCODE_F, false},
			{KeyCode::KEYCODE_G, false},
			{KeyCode::KEYCODE_H, false},
			{KeyCode::KEYCODE_J, false},
			{KeyCode::KEYCODE_K, false},
			{KeyCode::KEYCODE_L, false},
			{KeyCode::KEYCODE_Z, false},
			{KeyCode::KEYCODE_X, false},
			{KeyCode::KEYCODE_C, false},
			{KeyCode::KEYCODE_V, false},
			{KeyCode::KEYCODE_B, false},
			{KeyCode::KEYCODE_N, false},
			{KeyCode::KEYCODE_M, false},
			{KeyCode::KEYCODE_TAB, false},
			{KeyCode::KEYCODE_ENTER, false},
			{KeyCode::KEYCODE_LSHIFT, false},
			{KeyCode::KEYCODE_RSHIFT, false},
			{KeyCode::KEYCODE_LALT, false},
			{KeyCode::KEYCODE_RALT, false},
			{KeyCode::KEYCODE_ESCAPE, false}
		};
	};
}

#endif // INPUT_MANAGER_H
