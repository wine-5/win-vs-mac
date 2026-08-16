#include "UiInputMapper.h"
#include "core/input/KeyCode.h"

namespace game::ui
{
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
	}

	void UiInputMapper::update(float deltaTime)
	{
		// 上下左右は WASD でも操作できるようにする。移動と同じ指の形のまま
		// メニューを触れるほうが、持ち替えが要らず速い
		updateAction(UiAction::NavigateUp,
		    m_inputProvider.isKeyDown(KeyCode::Up) || m_inputProvider.isKeyDown(KeyCode::W), deltaTime, true);
		updateAction(UiAction::NavigateDown,
		    m_inputProvider.isKeyDown(KeyCode::Down) || m_inputProvider.isKeyDown(KeyCode::S), deltaTime, true);
		updateAction(UiAction::NavigateLeft,
		    m_inputProvider.isKeyDown(KeyCode::Left) || m_inputProvider.isKeyDown(KeyCode::A), deltaTime, true);
		updateAction(UiAction::NavigateRight,
		    m_inputProvider.isKeyDown(KeyCode::Right) || m_inputProvider.isKeyDown(KeyCode::D), deltaTime, true);

		// 決定と取り消しは繰り返さない。押しっぱなしで連続確定されると事故になる
		updateAction(UiAction::Confirm, m_inputProvider.isKeyDown(KeyCode::Enter), deltaTime, false);
		updateAction(UiAction::Cancel, m_inputProvider.isKeyDown(KeyCode::Escape), deltaTime, false);

		// キー操作があればフォーカス枠を出す
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

	bool UiInputMapper::isTriggered(UiAction action) const noexcept
	{
		return m_isTriggered[static_cast<int>(action)];
	}
} // namespace game::ui
