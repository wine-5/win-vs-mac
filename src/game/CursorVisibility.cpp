#include "CursorVisibility.h"

namespace game
{
	void CursorVisibility::setNeeded(Reason reason, bool isNeeded) noexcept
	{
		m_needed[static_cast<int>(reason)] = isNeeded;
	}

	void CursorVisibility::setPointerDrivenByPad(bool isDriven) noexcept
	{
		m_isPointerDrivenByPad = isDriven;
	}

	bool CursorVisibility::shouldShow(const core::iface::IInputProvider& inputProvider) const
	{
		bool isNeeded{ false };
		for (int i{ 0 }; i < REASON_COUNT && !isNeeded; ++i)
			isNeeded = m_needed[i];

		if (!isNeeded)
			return false;

		// パッドでカーソルそのものを動かす画面（セレクト）は、パッドを触っていても隠さない
		if (m_isPointerDrivenByPad)
			return true;

		// パッドを触っている間だけ隠す。接続の有無ではなく最後に触った機器で決めるのは、
		// パッドを挿したままマウスで遊ぶ人がいるため。抜かれたときも判定が
		// キーボード側へ落ちるので、そのまま出る
		return inputProvider.getLastInputDevice() != core::input::InputDevice::GamePad;
	}

	void CursorVisibility::update(core::iface::IInputProvider& inputProvider)
	{
		const bool isVisible{ shouldShow(inputProvider) };
		if (isVisible == m_isVisible)
			return;

		m_isVisible = isVisible;
		inputProvider.setMouseCursorVisible(isVisible);
	}
} // namespace game
