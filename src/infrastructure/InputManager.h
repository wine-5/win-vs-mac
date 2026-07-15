#pragma once
#include "core/interface/IInputProvider.h"
#include <unordered_map>

namespace infrastructure
{
	/**
	 * @brief DxLibを使ってキー入力を取得するクラス
	 */
	class InputManager : public core::iface::IInputProvider
	{
	public:
		InputManager();
		// ========== キーボード入力 ==========

		/**
		 * @brief 指定したキーが押されているか判定する
		 * @param keyCode キーコード
		 * @return 押されている場合true
		 */
		[[nodiscard]] bool isKeyDown(core::input::KeyCode keyCode) const override;
		
		/**
		 * @brief 指定したキーが押された瞬間か判定する（押しっぱなしは無視）
		 * @param keycode キーコード
		 * @return 押された瞬間の場合true
		 */
		[[nodiscard]] bool isKeyPressed(core::input::KeyCode keycode) const override;

		/**
		 * @brief フレームの最後に呼び出して前フレームの状態を更新する
		 */
		void updatePreviousState() override;

		// ========== ゲームパッド入力 ==========

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

		// ========== マウス入力 ==========

		/**
		 * @brief マウスの座標を取得する
		 * @param outX X座標の出力先
		 * @param outY Y座標の出力先
		 */
		void getMousePosition(int& outX, int& outY) const override;

		/**
		 * @brief マウスの左ボタンが押されているか判定する
		 * @return 押されている場合true
		 */
		[[nodiscard]] bool isMouseLeftPressed() const override;

		/**
		 * @brief マウスの右ボタンが押されているか判定する
		 * @return 押されている場合true
		 */
		[[nodiscard]] bool isMouseRightPressed() const override;

		/**
		 * @brief 前回取得時からのマウス移動量を取得する（取得後カーソルを画面中央へ戻す）
		 * @param outDx X方向の移動量の出力先
		 * @param outDy Y方向の移動量の出力先
		 */
		void getMouseDelta(int& outDx, int& outDy) override;

		/**
		 * @brief マウスカーソルの表示・非表示を切り替える
		 * @param visible trueで表示、falseで非表示
		 */
		void setMouseCursorVisible(bool visible) override;

	  private:
		mutable std::unordered_map<core::input::KeyCode, bool> m_previousKeyState;
	};
} // namespace infrastructure
