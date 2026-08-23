#pragma once

namespace core::input
{
	/**
	 * @brief いま操作に使っている入力機器の種類
	 *
	 * 操作の案内をどちらの表記で出すか（F2 なのか □ なのか）を決めるのに使う。
	 * パッドで遊んでいる人にキーボードのキー名を出し続けると、
	 * 画面に書いてあるのに何を押せばいいのか分からない状態になる
	 */
	enum class InputDevice
	{
		KeyboardMouse,
		GamePad,
	};
} // namespace core::input
