#pragma once
#include "core/interface/IInputProvider.h"

namespace game::ui
{
	/**
	 * @brief UI 操作の意図
	 *
	 * 「どのキーが押されたか」ではなく「何をしたいか」で表す。
	 * UI 側をこの単位で書いておけば、ゲームパッド対応のときに
	 * 割り当てを UiInputMapper へ足すだけで全画面が同時に対応できる
	 */
	enum class UiAction
	{
		NavigateUp,
		NavigateDown,
		NavigateLeft,
		NavigateRight,
		Confirm,
		Cancel,
		Count,
	};

	/**
	 * @brief キー入力を UI の意図（UiAction）へ翻訳するクラス
	 *
	 * 長押しの繰り返しと「最後に使った入力デバイス」の判定もここで引き受ける。
	 * 各 UI が自前でエッジ検出やリピートを書くと、画面ごとに反応の速さが変わってしまう
	 */
	class UiInputMapper
	{
	  public:
		/**
		 * @brief UiInputMapperのコンストラクタ
		 * @param inputProvider 入力のインターフェース
		 */
		explicit UiInputMapper(core::iface::IInputProvider& inputProvider);

		/**
		 * @brief このフレームの操作を読み取る（1フレームに1回だけ呼ぶ）
		 * @param deltaTime フレーム間の時間差（秒）
		 */
		void update(float deltaTime);

		/**
		 * @brief 内部状態を初期化する（画面を開いた直後に呼ぶ）
		 *
		 * これを呼ばないと、開くのに使ったキーの押しっぱなしが
		 * そのまま開いた先の操作として拾われてしまう
		 */
		void reset() noexcept;

		/**
		 * @brief この操作がこのフレームで成立したかを返す
		 * @param action 判定する操作
		 * @return 成立していれば true
		 */
		[[nodiscard]] bool isTriggered(UiAction action) const noexcept;

		/**
		 * @brief フォーカス枠を表示すべきかを返す
		 *
		 * Windows と同じく、キーボード・パッドを触った時だけ枠を出し、
		 * マウスを動かしたら消す。マウス操作中に枠が残っていると、
		 * いまどちらで操作しているのか分からなくなる
		 * @return 表示すべきなら true
		 */
		[[nodiscard]] bool isFocusVisible() const noexcept
		{
			return m_isFocusVisible;
		}

	  private:
		/** @brief 押しっぱなしで繰り返しが始まるまでの待ち時間（秒） */
		static constexpr float REPEAT_DELAY{ 0.4f };
		/** @brief 繰り返しの間隔（秒） */
		static constexpr float REPEAT_INTERVAL{ 0.08f };

		/** @brief スティックを「倒した」とみなす倒し量 */
		static constexpr float STICK_TRIGGER{ 0.5f };
		/** @brief スティックを「戻した」とみなす倒し量（成立より低くして往復を防ぐ） */
		static constexpr float STICK_RELEASE{ 0.35f };

		static constexpr int ACTION_COUNT{ static_cast<int>(UiAction::Count) };

		/** @brief スティックをデジタルの方向として覚えておく順番 */
		enum class StickDirection
		{
			Up,
			Down,
			Left,
			Right,
			Count,
		};

		static constexpr int STICK_DIRECTION_COUNT{ static_cast<int>(StickDirection::Count) };

		/**
		 * @brief 押されている状態から成立判定を作る
		 * @param action 判定する操作
		 * @param isDown いま押されているか
		 * @param deltaTime フレーム間の時間差（秒）
		 * @param allowRepeat 押しっぱなしで繰り返すか（決定・取り消しは繰り返さない）
		 */
		void updateAction(UiAction action, bool isDown, float deltaTime, bool allowRepeat);

		/**
		 * @brief スティックの倒し量をデジタルの方向として読む
		 *
		 * 1つのしきい値で切ると、境目で倒しているときに成立と解除を細かく繰り返して
		 * カーソルが暴れる。成立と解除で別のしきい値を使って落ち着かせる
		 * @param direction 判定する方向
		 * @param amount その方向への倒し量（0.0f〜1.0f）
		 * @return 倒していると見なすなら true
		 */
		[[nodiscard]] bool readStick(StickDirection direction, float amount) noexcept;

		core::iface::IInputProvider& m_inputProvider;

		bool m_isTriggered[ACTION_COUNT]{};
		bool m_wasDown[ACTION_COUNT]{};
		float m_repeatTimer[ACTION_COUNT]{};

		bool m_isFocusVisible{ false };

		// スティックを倒しているか（方向ごと）。しきい値の往復を抑えるために持つ
		bool m_stickDirections[STICK_DIRECTION_COUNT]{};

		// マウスが動いたかを見るために前フレームの座標を持つ
		int m_previousMouseX{ -1 };
		int m_previousMouseY{ -1 };
	};
} // namespace game::ui
