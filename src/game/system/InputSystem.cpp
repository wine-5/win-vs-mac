#include "InputSystem.h"
#include "core/KeyCode.h"
#include "game/component/InputComponent.h"

namespace game::system
{
	InputSystem::InputSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId playerId ,core::iface::IInputProvider& inputProvider)
		: m_componentManager(componentManager)
		, m_playerId(playerId)
		, m_inputProvider(inputProvider)
	{
	}

	void InputSystem::update(float deltaTime)
	{
		auto& input = m_componentManager.get<component::InputComponent>(m_playerId);

		// キーを離したときに前フレームの値が残らないように初期化
		input.m_moveX = INPUT_NEUTRAL;
		input.m_moveZ = INPUT_NEUTRAL;
		input.m_jumpPressed = false;

		if (m_inputProvider.isKeyDown(core::KeyCode::D)) input.m_moveX = INPUT_POSITIVE;
		if (m_inputProvider.isKeyDown(core::KeyCode::A)) input.m_moveX = INPUT_NEGATIVE;
		if (m_inputProvider.isKeyDown(core::KeyCode::W)) input.m_moveZ = INPUT_POSITIVE;
		if (m_inputProvider.isKeyDown(core::KeyCode::S)) input.m_moveZ = INPUT_NEGATIVE;
		if (m_inputProvider.isKeyDown(core::KeyCode::Space)) input.m_jumpPressed = true;
	}
}