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
		 * @brief フレームの最初に呼び出し、このフレームで使うキー状態を確定させる（スナップショット取得）
		 * @details 呼び出し後、次にこれが呼ばれるまでの間、isKeyDown/isKeyPressedは常に
		 * 同じ値を返す。これが無いと、フレーム内の複数箇所（Application・各Scene・各System）
		 * でキー入力をチェックした際に、その間にキーの状態が変化してしまい、
		 * isKeyPressedのエッジ検出が1フレーム分ズレて取りこぼされることがある
		 * （例：Escでポーズを開こうとしても、押した瞬間が別のチェック処理と重なると反応しない）。
		 */
		virtual void captureFrameInput() = 0;

		/**
		 * @brief 指定したキーが押されているか判定する
		 * @details 直近の captureFrameInput() 時点のスナップショットを参照する
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
		 * @brief 押された瞬間かを判定し、そのフレームぶんを消費する（1フレームに1回だけ成立）
		 *
		 * Applicationは1フレームに複数回 update を回すことがある（処理落ちの取り戻し）。
		 * 前回状態の更新はフレームに1回しか行わないため、isKeyPressed は同じ1押しに対して
		 * update の回数だけ true を返す。開閉のように「押すたびに1回だけ起こしたい」操作は
		 * これを使う。押しっぱなしの扱いは isKeyPressed と同じ
		 * @param keycode キーコード
		 * @return このフレームでまだ消費されていない「押された瞬間」ならtrue
		 */
		virtual bool consumeKeyPress(core::input::KeyCode keycode) = 0;

		/**
		 * @brief フレーム最後に呼び出して前フレームの入力状態を更新する
		 */
		virtual void updatePreviousState() = 0;

		// ========== ゲームパッド入力 ==========

		/**
		 * @brief 指定したゲームパッドボタンが押されているか判定する
		 * @details キーボードと同じく、直近の captureFrameInput() 時点のスナップショットを参照する
		 * @param code ゲームパッドコード
		 * @return 押されている場合true
		 */
		virtual bool isPadButtonDown(core::input::GamePadCode code) const = 0;

		/**
		 * @brief 指定したゲームパッドボタンが押された瞬間か判定する（押しっぱなしは無視）
		 * @param code ゲームパッドコード
		 * @return 押された瞬間の場合true
		 */
		virtual bool isPadButtonPressed(core::input::GamePadCode code) const = 0;

		/**
		 * @brief 押された瞬間かを判定し、そのフレームぶんを消費する（1フレームに1回だけ成立）
		 *
		 * 理由は consumeKeyPress と同じ。開閉のように「押すたびに1回だけ起こしたい」操作に使う
		 * @param code ゲームパッドコード
		 * @return このフレームでまだ消費されていない「押された瞬間」ならtrue
		 */
		virtual bool consumePadPress(core::input::GamePadCode code) = 0;

		/**
		 * @brief ゲームパッドのアナログ値を取得する
		 * @param code ゲームパッドコード
		 * @return アナログ値（スティックは-1.0f〜1.0f、トリガーは0.0f〜1.0f）
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