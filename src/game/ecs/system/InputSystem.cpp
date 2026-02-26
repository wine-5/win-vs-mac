#include "InputSystem.h"
#include <DxLib.h>
#include "game/ecs/component/InputComponent.h"

namespace game::ecs::system
{
	InputSystem::InputSystem(ComponentManager& componentManager, EntityId playerId)
		: m_componentManager(componentManager)
		, m_playerId(playerId)
	{
	}

	void InputSystem::update(float deltaTime)
	{
		auto& input = m_componentManager.get<component::InputComponent>(m_playerId);

		// キーを離したときに前フレームの値が残らないように初期化
		input.m_moveX = INPUT_NEUTRAL;
		input.m_moveZ = INPUT_NEUTRAL;
		input.m_jumpPressed = false;

		if (CheckHitKey(KEY_INPUT_D)) input.m_moveX = INPUT_POSITIVE;
		if (CheckHitKey(KEY_INPUT_A)) input.m_moveX = INPUT_NEGATIVE;
		if (CheckHitKey(KEY_INPUT_W)) input.m_moveZ = INPUT_POSITIVE;
		if (CheckHitKey(KEY_INPUT_S)) input.m_moveZ = INPUT_NEGATIVE;
		if (CheckHitKey(KEY_INPUT_SPACE)) input.m_jumpPressed = true;
	}
}