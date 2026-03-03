#include "InputManager.h"
#include "core/KeyCode.h"
#include <DxLib.h>
#include <unordered_map>

namespace infrastructure
{
	bool InputManager::isKeyDown(core::KeyCode keyCode) const
	{
		static const std::unordered_map<core::KeyCode, int> keyMap =
		{
			{core::KeyCode::W, KEY_INPUT_W},
			{core::KeyCode::A, KEY_INPUT_A},
			{core::KeyCode::S, KEY_INPUT_S},
			{core::KeyCode::D, KEY_INPUT_D},
			{core::KeyCode::Space, KEY_INPUT_SPACE},
		};

		auto it = keyMap.find(keyCode);
		if (it == keyMap.end()) return false;
		return CheckHitKey(it->second) != 0;
	}
}