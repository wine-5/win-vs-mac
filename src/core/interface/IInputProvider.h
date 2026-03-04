#pragma once
#include "core/input/KeyCode.h"
#include "core/input/GamePadCode.h"

namespace core::iface
{
	/**
	 * @brief キー入力取得の純粋仮想クラス
	 * Game層がInfrastructure層（DxLib）に直接依存しないための抽象化
	 */
	class IInputProvider
	{
	public:
		virtual ~IInputProvider() = default;

		// キーボード
		virtual bool isKeyDown(core::input::KeyCode keycode) const = 0;

		// コントローラーボタン
		virtual bool isPadButtonDown(core::input::GamePadCode code) const = 0;

		// コントローラースティック
		virtual float getPadAxis(core::input::GamePadCode code) const = 0;

		// コントローラーが接続されているか
		virtual bool isPadConnected() const = 0;
	};
}