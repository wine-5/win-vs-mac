#include "InputManager.h"
#include "core/input/KeyCode.h"
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
		};

		auto it = keyMap.find(keyCode);
		if (it == keyMap.end()) return false;
		return CheckHitKey(it->second) != 0;
	}
}