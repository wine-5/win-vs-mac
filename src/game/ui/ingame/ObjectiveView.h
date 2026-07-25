#pragma once
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "HudPanel.h"
#include <chrono>
#include <string>

namespace core::iface
{
	class IStringConverter; // 前方宣言
} // namespace core::iface

namespace game::ui::ingame
{
	/**
	 * @brief インゲーム左上の目標表示（残り雑魚の数・ボス出現）を描画するView
	 *
	 * デザインはWindows 11（Fluent）に準拠し、角丸8pxの半透明パネルに載せる。
	 * 「あと何体倒せば次に進めるのか」はプレイ中の行動を決める情報なので常時表示する。
	 *
	 * レイアウトの数値はすべて1080p基準で持ち、画面高さに応じて拡大縮小する
	 */
	class ObjectiveView
	{
	  public:
		/**
		 * @brief ObjectiveViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 */
		ObjectiveView(core::iface::IUIRenderer& uiRenderer, core::iface::IScreen& screen);

		/**
		 * @brief 目標を描画する
		 * @param remainingEnemyCount 残っている開始時配置の雑魚の数
		 * @param isBossAppeared ボスが出現済みかどうか（trueなら討伐目標に切り替える）
		 */
		void draw(int remainingEnemyCount, bool isBossAppeared);

	  private:
		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		/**
		 * @brief 残り数の変化を検知し、減ったときに反応アニメーションを開始する
		 * @param remainingEnemyCount 今フレームの残り数
		 */
		void updateCountReaction(int remainingEnemyCount);

		/**
		 * @brief 反応アニメーションの進行度を返す
		 * @return 0.0（開始直後）〜1.0（終了）。再生中でなければ1.0
		 */
		[[nodiscard]] float getCountReactionProgress() const;

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		HudPanel m_panel;

		// DxLibの描画はShift_JISを期待するため、ソース上のUTF-8日本語をそのまま渡すと文字化けする。
		// 変換結果は毎フレーム同じなので生成時に一度だけ変換して保持する
		std::string m_detailText{};
		std::string m_bossText{};

		// 残り数が減った瞬間に反応させるための状態。
		// 描画経路からしか呼ばれずdeltaTimeを受け取らないため、経過時間は壁時計から求める
		int m_lastCount{ -1 };
		std::chrono::steady_clock::time_point m_countChangedTime{};
		bool m_isCountReacting{ false };
	};
} // namespace game::ui::ingame
