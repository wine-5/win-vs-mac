#include "UiInputMapper.h"
#include "core/input/KeyCode.h"
#include "core/input/GamePadCode.h"

namespace game::ui
{
	using core::input::GamePadCode;
	using core::input::KeyCode;

	UiInputMapper::UiInputMapper(core::iface::IInputProvider& inputProvider)
	    : m_inputProvider{ inputProvider }
	{
	}

	void UiInputMapper::reset() noexcept
	{
		for (int i{ 0 }; i < ACTION_COUNT; ++i)
		{
			m_isTriggered[i] = false;
			m_repeatTimer[i] = 0.0f;

			// 開くのに使ったキーがまだ押されている前提で「押されていた」ことにする。
			// こうしておくと離して押し直すまで成立しない
			m_wasDown[i] = true;
		}

		m_isFocusVisible = false;
		m_previousMouseX = -1;
		m_previousMouseY = -1;

		for (int i{ 0 }; i < STICK_DIRECTION_COUNT; ++i)
			m_stickDirections[i] = false;
	}

	void UiInputMapper::update(float deltaTime)
	{
		// パッドの左スティックはアナログなので、方向ごとにデジタルへ落としてから
		// キーや十字キーと同じ扱いで流す
		const float stickX{ m_inputProvider.getPadAxis(GamePadCode::LeftStickX) };
		const float stickY{ m_inputProvider.getPadAxis(GamePadCode::LeftStickY) };
		const bool stickUp{ readStick(StickDirection::Up, stickY) };
		const bool stickDown{ readStick(StickDirection::Down, -stickY) };
		const bool stickLeft{ readStick(StickDirection::Left, -stickX) };
		const bool stickRight{ readStick(StickDirection::Right, stickX) };

		// 上下左右は WASD でも操作できるようにする。移動と同じ指の形のまま
		// メニューを触れるほうが、持ち替えが要らず速い
		updateAction(UiAction::NavigateUp,
		    m_inputProvider.isKeyDown(KeyCode::Up) || m_inputProvider.isKeyDown(KeyCode::W) ||
		        m_inputProvider.isPadButtonDown(GamePadCode::DPadUp) || stickUp,
		    deltaTime, true);
		updateAction(UiAction::NavigateDown,
		    m_inputProvider.isKeyDown(KeyCode::Down) || m_inputProvider.isKeyDown(KeyCode::S) ||
		        m_inputProvider.isPadButtonDown(GamePadCode::DPadDown) || stickDown,
		    deltaTime, true);
		updateAction(UiAction::NavigateLeft,
		    m_inputProvider.isKeyDown(KeyCode::Left) || m_inputProvider.isKeyDown(KeyCode::A) ||
		        m_inputProvider.isPadButtonDown(GamePadCode::DPadLeft) || stickLeft,
		    deltaTime, true);
		updateAction(UiAction::NavigateRight,
		    m_inputProvider.isKeyDown(KeyCode::Right) || m_inputProvider.isKeyDown(KeyCode::D) ||
		        m_inputProvider.isPadButtonDown(GamePadCode::DPadRight) || stickRight,
		    deltaTime, true);

		// 決定と取り消しは繰り返さない。押しっぱなしで連続確定されると事故になる。
		// パッドは×が決定、〇が取り消し。PCで多数派のXbox配列に合わせて下のボタンを決定にする
		updateAction(UiAction::Confirm,
		    m_inputProvider.isKeyDown(KeyCode::Enter) ||
		        m_inputProvider.isPadButtonDown(GamePadCode::ButtonCross),
		    deltaTime, false);
		updateAction(UiAction::Cancel,
		    m_inputProvider.isKeyDown(KeyCode::Escape) ||
		        m_inputProvider.isPadButtonDown(GamePadCode::ButtonCircle),
		    deltaTime, false);

		// キー・パッド操作があればフォーカス枠を出す
		for (int i{ 0 }; i < ACTION_COUNT; ++i)
		{
			if (!m_isTriggered[i])
				continue;

			m_isFocusVisible = true;
			break;
		}

		// マウスが動いたらフォーカス枠を消す
		int mouseX{}, mouseY{};
		m_inputProvider.getMousePosition(mouseX, mouseY);
		if (m_previousMouseX >= 0 && (mouseX != m_previousMouseX || mouseY != m_previousMouseY))
			m_isFocusVisible = false;

		m_previousMouseX = mouseX;
		m_previousMouseY = mouseY;
	}

	void UiInputMapper::updateAction(UiAction action, bool isDown, float deltaTime, bool allowRepeat)
	{
		const int index{ static_cast<int>(action) };
		m_isTriggered[index] = false;

		if (!isDown)
		{
			m_wasDown[index] = false;
			m_repeatTimer[index] = 0.0f;
			return;
		}

		if (!m_wasDown[index])
		{
			// 押した瞬間。ここから REPEAT_DELAY 待ってから繰り返しを始める
			m_wasDown[index] = true;
			m_isTriggered[index] = true;
			m_repeatTimer[index] = REPEAT_DELAY;
			return;
		}

		if (!allowRepeat)
			return;

		m_repeatTimer[index] -= deltaTime;
		if (m_repeatTimer[index] > 0.0f)
			return;

		m_isTriggered[index] = true;
		m_repeatTimer[index] = REPEAT_INTERVAL;
	}

	bool UiInputMapper::readStick(StickDirection direction, float amount) noexcept
	{
		const int index{ static_cast<int>(direction) };
		const float threshold{ m_stickDirections[index] ? STICK_RELEASE : STICK_TRIGGER };
		m_stickDirections[index] = amount >= threshold;
		return m_stickDirections[index];
	}

	bool UiInputMapper::isTriggered(UiAction action) const noexcept
	{
		return m_isTriggered[static_cast<int>(action)];
	}
} // namespace game::ui
