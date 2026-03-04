#include "InputManager.h"
#include <DxLib.h>
#include <unordered_map>

namespace infrastructure
{
	bool InputManager::isKeyDown(core::input::KeyCode keyCode) const
	{
		static const std::unordered_map<core::input::KeyCode, int> keyMap =
		{
			{core::input::KeyCode::W, KEY_INPUT_W},
			{core::input::KeyCode::A, KEY_INPUT_A},
			{core::input::KeyCode::S, KEY_INPUT_S},
			{core::input::KeyCode::D, KEY_INPUT_D},
			{core::input::KeyCode::Space, KEY_INPUT_SPACE},

			{ core::input::KeyCode::Up,    KEY_INPUT_UP    },
			{ core::input::KeyCode::Down,  KEY_INPUT_DOWN  },
			{ core::input::KeyCode::Left,  KEY_INPUT_LEFT  },
			{ core::input::KeyCode::Right, KEY_INPUT_RIGHT },
		};

		auto it = keyMap.find(keyCode);
		if (it == keyMap.end()) return false;
		return CheckHitKey(it->second) != 0;
	}

	bool InputManager::isPadButtonDown(core::input::GamePadCode code) const
	{
		int input = GetJoypadInputState(DX_INPUT_PAD1);
		switch (code)
		{
		case core::input::GamePadCode::ButtonB:  return (input & PAD_INPUT_B) != 0;
		case core::input::GamePadCode::DPadUp:   return (input & PAD_INPUT_UP) != 0;
		case core::input::GamePadCode::DPadDown:  return (input & PAD_INPUT_DOWN) != 0;
		case core::input::GamePadCode::DPadLeft:  return (input & PAD_INPUT_LEFT) != 0;
		case core::input::GamePadCode::DPadRight: return (input & PAD_INPUT_RIGHT) != 0;
		default: return false;
		}
	}

	float InputManager::getPadAxis(core::input::GamePadCode code) const
	{
		int x = 0, y = 0;
		GetJoypadAnalogInput(&x, &y, DX_INPUT_PAD1);

		switch (code)
		{
		case core::input::GamePadCode::LeftStickX: return x / 1000.0f;
		case core::input::GamePadCode::LeftStickY: return y / 1000.0f;
		default: return 0.0f;
		}
	}

	bool InputManager::isPadConnected() const
	{
		return GetJoypadInputState(DX_INPUT_PAD1) != -1;
	}
}