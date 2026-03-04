#include "InputSystem.h"
#include "core/input/KeyCode.h"
#include "core/input/GamePadCode.h"
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

		// キーボード入力
		if (m_inputProvider.isKeyDown(core::input::KeyCode::D) || m_inputProvider.isKeyDown(core::input::KeyCode::Right))
			input.m_moveX = INPUT_POSITIVE;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::A) || m_inputProvider.isKeyDown(core::input::KeyCode::Left))
			input.m_moveX = INPUT_NEGATIVE;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::W) || m_inputProvider.isKeyDown(core::input::KeyCode::Up))
			input.m_moveZ = INPUT_POSITIVE;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::S) || m_inputProvider.isKeyDown(core::input::KeyCode::Down))
			input.m_moveZ = INPUT_NEGATIVE;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::Space))
			input.m_jumpPressed = true;

		if (!m_inputProvider.isPadConnected()) return;

		// コントローラースティック入力
		float axisX = m_inputProvider.getPadAxis(core::input::GamePadCode::LeftStickX);
		float axisY = m_inputProvider.getPadAxis(core::input::GamePadCode::LeftStickY);
		if (axisX != 0.0f)
			input.m_moveX = axisX;
		if (axisY != 0.0f)
			input.m_moveZ = axisY;

		// コントローラー十字・ボタン入力
		if(m_inputProvider.isPadButtonDown(core::input::GamePadCode::DPadRight))
			input.m_moveX = INPUT_POSITIVE;
		if (m_inputProvider.isPadButtonDown(core::input::GamePadCode::DPadLeft))
			input.m_moveX = INPUT_NEGATIVE;
		if (m_inputProvider.isPadButtonDown(core::input::GamePadCode::DPadUp))
			input.m_moveZ = INPUT_POSITIVE;
		if (m_inputProvider.isPadButtonDown(core::input::GamePadCode::DPadDown))
			input.m_moveZ = INPUT_NEGATIVE;
		if (m_inputProvider.isPadButtonDown(core::input::GamePadCode::ButtonB))
			input.m_jumpPressed = true;
	}
}