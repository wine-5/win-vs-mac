#include "InputSystem.h"
#include "core/input/KeyCode.h"
#include "core/input/GamePadCode.h"
#include "game/component/InputComponent.h"
#include "game/GameManager.h"

namespace game::system
{
	InputSystem::InputSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId, core::iface::IInputProvider& inputProvider, GameManager& gameManager)
	    : m_componentManager{ componentManager }
	    , m_entityId{ entityId }
	    , m_inputProvider{ inputProvider }
	    , m_gameManager{ gameManager }
	{
	}

	void InputSystem::update(float deltaTime)
	{
		auto& input = m_componentManager.get<component::InputComponent>(m_entityId);

		// キーを離したときに前フレームの値が残らないように初期化
		input.m_moveX = INPUT_NEUTRAL;
		input.m_moveZ = INPUT_NEUTRAL;
		input.m_jumpPressed = false;
		input.m_attackPressed = false;
		input.m_dashPressed = false;
		input.m_rangedAttackPressed = false;

		// シネマ演出中（ボス覚醒など）は全入力を受け付けない（上の初期化でニュートラルを維持）
		if (input.m_locked)
			return;

		// DEBUG: デバッグモード中はWASD/Space/Shift/マウスをフリーカメラが使うため、
		// Playerの移動は矢印キーのみで行う（リリース時に削除）
		const bool debugMode{ m_gameManager.isDebugMode() };
		const bool allowWasd{ !debugMode };

		// -------------------------------------------------------
		// PCでの操作の入力
		// -------------------------------------------------------

		// キー入力（矢印キーは常時有効、WASDはデバッグ中のみ無効）
		if ((allowWasd && m_inputProvider.isKeyDown(core::input::KeyCode::D)) || m_inputProvider.isKeyDown(core::input::KeyCode::Right))
			input.m_moveX = INPUT_POSITIVE;
		if ((allowWasd && m_inputProvider.isKeyDown(core::input::KeyCode::A)) || m_inputProvider.isKeyDown(core::input::KeyCode::Left))
			input.m_moveX = INPUT_NEGATIVE;
		if ((allowWasd && m_inputProvider.isKeyDown(core::input::KeyCode::W)) || m_inputProvider.isKeyDown(core::input::KeyCode::Up))
			input.m_moveZ = INPUT_POSITIVE;
		if ((allowWasd && m_inputProvider.isKeyDown(core::input::KeyCode::S)) || m_inputProvider.isKeyDown(core::input::KeyCode::Down))
			input.m_moveZ = INPUT_NEGATIVE;
		// Space（ジャンプ）/Shift（ダッシュ）/マウス攻撃はデバッグ中はカメラ操作に使うため無効化する
		if (allowWasd && m_inputProvider.isKeyDown(core::input::KeyCode::Space))
			input.m_jumpPressed = true;
		if (allowWasd && m_inputProvider.isKeyDown(core::input::KeyCode::Shift))
			input.m_dashPressed = true;

		// マウスの入力
		if (m_inputProvider.isMouseLeftPressed())
			input.m_attackPressed = true;
		if (m_inputProvider.isMouseRightPressed())
			input.m_rangedAttackPressed = true;

		if (!m_inputProvider.isPadConnected()) return;

		// -------------------------------------------------------
		// コントローラーの入力
		// -------------------------------------------------------
		
		// スティック入力
		float axisX{m_inputProvider.getPadAxis(core::input::GamePadCode::LeftStickX)};
		float axisY{m_inputProvider.getPadAxis(core::input::GamePadCode::LeftStickY)};
		if (axisX != 0.0f)
			input.m_moveX = axisX;
		if (axisY != 0.0f)
			input.m_moveZ = axisY;

		// 十字
		if(m_inputProvider.isPadButtonDown(core::input::GamePadCode::DPadRight))
			input.m_moveX = INPUT_POSITIVE;
		if (m_inputProvider.isPadButtonDown(core::input::GamePadCode::DPadLeft))
			input.m_moveX = INPUT_NEGATIVE;
		if (m_inputProvider.isPadButtonDown(core::input::GamePadCode::DPadUp))
			input.m_moveZ = INPUT_POSITIVE;
		if (m_inputProvider.isPadButtonDown(core::input::GamePadCode::DPadDown))
			input.m_moveZ = INPUT_NEGATIVE;

		// ボタン
		if (m_inputProvider.isPadButtonDown(core::input::GamePadCode::ButtonA))
			input.m_attackPressed = true;
		if (m_inputProvider.isPadButtonDown(core::input::GamePadCode::ButtonB))
			input.m_jumpPressed = true;
	}
} // namespace game::system