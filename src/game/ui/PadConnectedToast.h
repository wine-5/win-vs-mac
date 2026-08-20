#pragma once
#include "core/interface/IInputProvider.h"
#include "core/interface/IScreen.h"
#include "core/interface/IUIRenderer.h"
#include "game/ui/PadButtonIcon.h"
#include <string>

namespace game::ui
{
	/**
	 * @brief パッドが繋がっていることを右下で知らせるトーストView
	 *
	 * 起動時に既にパッドが挿さっていても、画面はマウス前提の見た目のままなので、
	 * パッドで操作してよいのかが分からない。Steamと同じく、繋がっていることを
	 * 一度だけ右下で伝えて引っ込める。
	 *
	 * 出るときは左から右へ滑り込み、消えるときは右から左へ戻る。
	 * 位置を変えずに濃さだけで出し入れすると、視界の端では出たことに気付けない
	 */
	class PadConnectedToast
	{
	  public:
		/**
		 * @brief PadConnectedToastのコンストラクタ
		 * @param inputProvider パッドの接続を見るのに使う
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 */
		PadConnectedToast(core::iface::IInputProvider& inputProvider,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen);

		/**
		 * @brief 接続の変化を見て、出し入れを進める
		 * @param deltaTime フレーム間の時間差（秒）
		 */
		void update(float deltaTime);

		/**
		 * @brief トーストを描画する（出ていなければ何もしない）
		 */
		void draw();

	  private:
		/** @brief 出し入れの段階 */
		enum class Phase
		{
			Hidden,   // 出ていない
			SlideIn,  // 左から右へ滑り込む
			Hold,     // 出したまま留める
			SlideOut, // 右から左へ戻りながら消える
		};

		/** @brief 滑り込みにかける時間（秒） */
		static constexpr float SLIDE_DURATION{ 0.28f };
		/** @brief 出したまま留める時間（秒） */
		static constexpr float HOLD_DURATION{ 3.2f };

		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		/**
		 * @brief いまの濃さ（0.0〜1.0）を返す
		 * @return 0.0（完全に消えている）〜1.0（出し切っている）
		 */
		[[nodiscard]] float visibility() const;

		core::iface::IInputProvider& m_inputProvider;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		PadButtonIcon m_padButtonIcon;

		Phase m_phase{ Phase::Hidden };
		float m_phaseTime{ 0.0f };

		// 前フレームの接続状態。差分を見て、挿し直したときにもう一度出せるようにする
		bool m_wasConnected{ false };

		// DxLibの描画はShift_JISを期待するため、生成時に一度だけ変換して持つ
		std::string m_message{};
	};
} // namespace game::ui
