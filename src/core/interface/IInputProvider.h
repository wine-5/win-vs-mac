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

		// ========== キーボード入力 ==========

		/**
		 * @brief 指定したキーが押されているか判定する
		 * @param keycode キーコード
		 * @return 押されている場合true
		 */
		virtual bool isKeyDown(core::input::KeyCode keycode) const = 0;

		/**
		 * @brief 指定したキーが押された瞬間か判定する（押しっぱなしは無視）
		 * @param keycode キーコード
		 * @return 押された瞬間の場合true
		 */
		virtual bool isKeyPressed(core::input::KeyCode keycode) const = 0;

		/**
		 * @brief フレーム最後に呼び出して前フレームの入力状態を更新する
		 */
		virtual void updatePreviousState() = 0;

		// ========== ゲームパッド入力 ==========

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

		// ========== マウス入力 ==========
		/**
		 * @brief マウスの座標を取得する
		 * @param outX X座標の出力先
		 * @param outY Y座標の出力先
		 */
		virtual void getMousePosition(int& outX, int& outY) const = 0;

		/**
		 * @brief マウスの左ボタンが押されているか判定する
		 * @return 押されている場合true
		 */
		virtual bool isMouseLeftPressed() const = 0;

		/**
		 * @brief マウスの右ボタンが押されているか判定する
		 * @return 押されている場合true
		 */
		virtual bool isMouseRightPressed() const = 0;

		/**
		 * @brief 前回取得時からのマウス移動量を取得する（取得後カーソルを画面中央へ戻す）
		 * @param outDx X方向の移動量の出力先
		 * @param outDy Y方向の移動量の出力先
		 */
		virtual void getMouseDelta(int& outDx, int& outDy) = 0;

		/**
		 * @brief マウスカーソルの表示・非表示を切り替える
		 * @param visible trueで表示、falseで非表示
		 */
		virtual void setMouseCursorVisible(bool visible) = 0;
	};
} // namespace core::iface