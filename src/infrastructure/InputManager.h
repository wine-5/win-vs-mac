#pragma once
#include "core/interface/IInputProvider.h"
#include <unordered_map>
#include <unordered_set>

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
		 * @brief フレームの最初に呼び出し、このフレームで使うキー状態を確定させる
		 * @details KEY_MAP内の全キーについて現在の状態をスナップショットに保存する。
		 * これにより、isKeyDown/isKeyPressedはこのフレーム中は常に同じ値を返す。
		 */
		void captureFrameInput() override;

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
		 * @brief 押された瞬間かを判定し、そのフレームぶんを消費する
		 * @param keycode キーコード
		 * @return このフレームでまだ消費されていない「押された瞬間」ならtrue
		 */
		[[nodiscard]] bool consumeKeyPress(core::input::KeyCode keycode) override;

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
		 * @brief 指定したゲームパッドボタンが押された瞬間か判定する
		 * @param code ゲームパッドコード
		 * @return 押された瞬間の場合true
		 */
		[[nodiscard]] bool isPadButtonPressed(core::input::GamePadCode code) const override;

		/**
		 * @brief 押された瞬間かを判定し、そのフレームぶんを消費する
		 * @param code ゲームパッドコード
		 * @return このフレームでまだ消費されていない「押された瞬間」ならtrue
		 */
		[[nodiscard]] bool consumePadPress(core::input::GamePadCode code) override;

		/**
		 * @brief ゲームパッドのアナログ値を取得する
		 * @param code ゲームパッドコード
		 * @return アナログ値（スティックは-1.0f〜1.0f、トリガーは0.0f〜1.0f）
		 */
		[[nodiscard]] float getPadAxis(core::input::GamePadCode code) const override;

		/**
		 * @brief ゲームパッドが接続されているか判定する
		 * @return 接続されている場合true
		 */
		[[nodiscard]] bool isPadConnected() const override;

		/**
		 * @brief 最後に操作へ使われた入力機器を返す
		 * @return 最後に触られた入力機器
		 */
		[[nodiscard]] core::input::InputDevice getLastInputDevice() const override;

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
		 * @details 他アプリが前面の間は 0 を返し、カーソルの位置にも触れない
		 * @param outDx X方向の移動量の出力先
		 * @param outDy Y方向の移動量の出力先
		 */
		void getMouseDelta(int& outDx, int& outDy) override;

		/**
		 * @brief マウスカーソルの表示・非表示を切り替える
		 * @param visible trueで表示、falseで非表示
		 */
		void setMouseCursorVisible(bool visible) override;

		/**
		 * @brief OSのマウスカーソルを動かす（パッドで画面を指すのに使う）
		 * @param deltaX 横方向の移動量（ピクセル）
		 * @param deltaY 縦方向の移動量（ピクセル）
		 */
		void movePointer(int deltaX, int deltaY) override;

		/**
		 * @brief いまカーソルがある位置へ左クリックを送る
		 */
		void clickPointer() override;

	  private:
		/**
		 * @brief パッドをどの経路から読んでいるか
		 *
		 * 素のDualShock 4／DualSenseはWindowsではXInputのデバイスとして見えず、
		 * GetJoypadXInputStateが-1を返す。その場合はDirectInputから読む
		 */
		enum class PadKind
		{
			None,
			XInput,
			DirectInput,
		};

		static constexpr int PAD_CODE_COUNT{ static_cast<int>(core::input::GamePadCode::Count) };

		/** @brief スティックの遊び。これ以下の倒し量は0として捨てる（据え置きのドリフト対策） */
		static constexpr float STICK_DEADZONE{ 0.24f };
		/** @brief トリガーを「押した」とみなす踏み込み量 */
		static constexpr float TRIGGER_THRESHOLD{ 0.5f };

		/**
		 * @brief このフレームで使うパッドの状態を確定させる
		 * @param isFocused 自分のアプリが前面か（前面でなければ全て未入力にする）
		 */
		void capturePadInput(bool isFocused);

		/**
		 * @brief XInputから状態を読む
		 * @return 読めたらtrue
		 */
		bool capturePadFromXInput();

		/**
		 * @brief DirectInputから状態を読む（DualShock 4／DualSenseの並びを前提にする）
		 * @return 読めたらtrue
		 */
		bool capturePadFromDirectInput();

		/**
		 * @brief スティックの倒し量へ円形の遊びを入れて格納する
		 *
		 * 軸ごとに切ると斜めの遊びが四角くなり、真横・真上へ寄りやすくなるため、
		 * 大きさで判定してから向きを保ったまま0〜1へ引き伸ばす
		 * @param xCode 横方向の格納先
		 * @param yCode 縦方向の格納先
		 * @param rawX 横方向の生の倒し量（-1.0f〜1.0f）
		 * @param rawY 縦方向の生の倒し量（-1.0f〜1.0f・上が正）
		 */
		void storeStick(core::input::GamePadCode xCode, core::input::GamePadCode yCode, float rawX, float rawY);

		/** @brief パッドの状態を全て未入力へ戻す */
		void clearPadState() noexcept;

		/**
		 * @brief パッドが1つも見つからないときに、次の探索まで空ける回数
		 *
		 * XInput は繋がっていないスロットへの問い合わせが重い。見つからないのに
		 * 毎回2つとも問い合わせると、その空振りだけで時間を取られる
		 */
		static constexpr int PAD_PROBE_INTERVAL{ 60 };

		/**
		 * @brief このフレームの入力から「最後に触った機器」を更新する
		 *
		 * パッドを優先して見るのは、パッドで遊んでいる最中に机の上のマウスが
		 * わずかに動いただけで表記が戻ってしまうのを避けるため。
		 * マウスの微動は下の MOUSE_MOVE_THRESHOLD で捨てる
		 */
		void updateLastInputDevice();

		/** @brief マウスが「動いた」とみなす移動量（ピクセル） */
		static constexpr int MOUSE_MOVE_THRESHOLD{ 3 };

		std::unordered_map<core::input::KeyCode, bool> m_currentKeyState;          // captureFrameInput()でキャプチャした今フレームの状態
		mutable std::unordered_map<core::input::KeyCode, bool> m_previousKeyState; // isKeyPressed(const)内でoperator[]により新規挿入されうる

		// このフレームで既に消費した「押された瞬間」。updatePreviousState で空にする
		std::unordered_set<core::input::KeyCode> m_consumedKeys;
		// 自分のアプリが前面か。captureFrameInput() で毎フレーム更新し、
		// キー入力だけでなくマウス視点操作の可否にも使う
		bool m_isWindowFocused{ true };

		bool m_cursorVisible{ true }; // 表示中は前回座標との差分、非表示中は中央固定差分を使う
		int m_previousMouseX{ 0 };
		int m_previousMouseY{ 0 };
		bool m_hasPreviousMousePosition{ false };

		// 前面へ戻った直後にカーソルを中央へ置き直すか。離れた位置のカーソルを
		// そのまま差分にすると視点が飛ぶため、1フレームぶん捨てるのに使う
		bool m_needsMouseRecenter{ false };

		// パッドもキーボードと同じくフレーム頭で確定させる。フレーム内の複数箇所で
		// 読んでもエッジ検出がずれないようにするため
		PadKind m_padKind{ PadKind::None };
		bool m_currentPadButtons[PAD_CODE_COUNT]{};
		bool m_previousPadButtons[PAD_CODE_COUNT]{};
		bool m_consumedPadButtons[PAD_CODE_COUNT]{}; // updatePreviousStateで空にする
		float m_padAxes[PAD_CODE_COUNT]{};

		// 最後に触られた入力機器。案内の表記を切り替えるのに使う。
		// マウスの微動を捨てるため、判定用に前フレームの座標を別に持つ
		// 次にパッドを探し直すまでの残り回数。見つからない間の空振りを減らすために使う
		int m_padProbeCountdown{ 0 };

		core::input::InputDevice m_lastInputDevice{ core::input::InputDevice::KeyboardMouse };
		int m_deviceMouseX{ 0 };
		int m_deviceMouseY{ 0 };
		bool m_hasDeviceMousePosition{ false };
	};
} // namespace infrastructure
