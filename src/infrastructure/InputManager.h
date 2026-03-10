#pragma once
#include "core/interface/IInputProvider.h"

namespace infrastructure
{
	/**
	 * @brief DxLibを使ってキー入力を取得するクラス
	 */
	class InputManager : public core::iface::IInputProvider
	{
	public:
		/**
		 * @brief 指定したキーが押されているか判定する
		 * @param keyCode キーコード
		 * @return 押されている場合true
		 */
		[[nodiscard]] bool isKeyDown(core::input::KeyCode keyCode) const override;
		
		/**
		 * @brief 指定したゲームパッドボタンが押されているか判定する
		 * @param code ゲームパッドコード
		 * @return 押されている場合true
		 */
		[[nodiscard]] bool isPadButtonDown(core::input::GamePadCode code) const override;
		
		/**
		 * @brief ゲームパッドのアナログ値を取得する
		 * @param code ゲームパッドコード
		 * @return アナログ値（-1.0f〜1.0f）
		 */
		[[nodiscard]] float getPadAxis(core::input::GamePadCode code) const override;
		
		/**
		 * @brief ゲームパッドが接続されているか判定する
		 * @return 接続されている場合true
		 */
		[[nodiscard]] bool isPadConnected() const override;
	};
}
