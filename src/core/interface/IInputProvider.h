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

		/**
		 * @brief 指定したキーが押されているか判定する
		 * @param keycode キーコード
		 * @return 押されている場合true
		 */
		virtual bool isKeyDown(core::input::KeyCode keycode) const = 0;

		/**
		 * @brief 指定したゲームパッドボタンが押されているか判定する
		 * @param code ゲームパッドコード
		 * @return 押されている場合true
		 */
		virtual bool isPadButtonDown(core::input::GamePadCode code) const = 0;

		/**
		 * @brief ゲームパッドのアナログ値を取得する
		 * @param code ゲームパッドコード
		 * @return アナログ値（-1.0f〜1.0f）
		 */
		virtual float getPadAxis(core::input::GamePadCode code) const = 0;

		/**
		 * @brief ゲームパッドが接続されているか判定する
		 * @return 接続されている場合true
		 */
		virtual bool isPadConnected() const = 0;
	};
}