#include "InputSystem.h"
#include "core/input/KeyCode.h"
#include "core/input/GamePadCode.h"
#include "game/component/movement/InputComponent.h"

namespace game::system::movement
{
	InputSystem::InputSystem(core::ecs::ComponentManager& componentManager, core::ecs::EntityId entityId, core::iface::IInputProvider& inputProvider)
	    : m_componentManager{ componentManager }
	    , m_entityId{ entityId }
	    , m_inputProvider{ inputProvider }
	{
	}

	void InputSystem::update(float deltaTime)
	{
		auto& input = m_componentManager.get<component::movement::InputComponent>(m_entityId);

		// キーを離したときに前フレームの値が残らないように初期化
		input.m_moveX = INPUT_NEUTRAL;
		input.m_moveZ = INPUT_NEUTRAL;
		input.m_jumpPressed = false;
		input.m_attackPressed = false;
		input.m_dashPressed = false;
		input.m_rangedAttackPressed = false;
		input.m_statusViewPressed = false;

		// シネマ演出中（ボス覚醒など）と、インベントリを開いている間は
		// 全入力を受け付けない（上の初期化でニュートラルを維持）
		if (input.isInputBlocked())
			return;

		// -------------------------------------------------------
		// PCでの操作の入力
		// -------------------------------------------------------

		// キー入力（WASDと矢印キーのどちらでも動かせる）
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
		if (m_inputProvider.isKeyDown(core::input::KeyCode::Shift))
			input.m_dashPressed = true;
		if (m_inputProvider.isKeyDown(core::input::KeyCode::Tab))
			input.m_statusViewPressed = true;

		// マウスの入力
		if (m_inputProvider.isMouseLeftPressed())
			input.m_attackPressed = true;
		if (m_inputProvider.isMouseRightPressed())
			input.m_rangedAttackPressed = true;

		if (!m_inputProvider.isPadConnected())
			return;

		// -------------------------------------------------------
		// コントローラーの入力
		// -------------------------------------------------------
		using core::input::GamePadCode;

		// 左スティック。遊びと正規化はInputManagerで済んでいるので、0でなければ倒している
		const float stickX{ m_inputProvider.getPadAxis(GamePadCode::LeftStickX) };
		const float stickY{ m_inputProvider.getPadAxis(GamePadCode::LeftStickY) };
		if (stickX != 0.0f)
			input.m_moveX = stickX;
		if (stickY != 0.0f)
			input.m_moveZ = stickY;

		// 十字キー。スティックと違い倒し量が無いので、キーボードと同じ扱いにする
		if (m_inputProvider.isPadButtonDown(GamePadCode::DPadRight))
			input.m_moveX = INPUT_POSITIVE;
		if (m_inputProvider.isPadButtonDown(GamePadCode::DPadLeft))
			input.m_moveX = INPUT_NEGATIVE;
		if (m_inputProvider.isPadButtonDown(GamePadCode::DPadUp))
			input.m_moveZ = INPUT_POSITIVE;
		if (m_inputProvider.isPadButtonDown(GamePadCode::DPadDown))
			input.m_moveZ = INPUT_NEGATIVE;

		// ×でジャンプ、〇で近接攻撃。近接はコンボの連打なので面ボタンへ置く
		if (m_inputProvider.isPadButtonDown(GamePadCode::ButtonCross))
			input.m_jumpPressed = true;
		if (m_inputProvider.isPadButtonDown(GamePadCode::ButtonCircle))
			input.m_attackPressed = true;

		// R1は押している間ためる遠距離攻撃。L1と左スティック押し込みはダッシュで、
		// どちらでも受けるのは持ち方によって押しやすい方が違うため
		if (m_inputProvider.isPadButtonDown(GamePadCode::ButtonR1))
			input.m_rangedAttackPressed = true;
		if (m_inputProvider.isPadButtonDown(GamePadCode::ButtonL1) ||
		    m_inputProvider.isPadButtonDown(GamePadCode::ButtonL3))
			input.m_dashPressed = true;

		// SHAREは押している間だけステータス一覧（Tabと同じ）
		if (m_inputProvider.isPadButtonDown(GamePadCode::ButtonShare))
			input.m_statusViewPressed = true;
	}
} // namespace game::system::movement