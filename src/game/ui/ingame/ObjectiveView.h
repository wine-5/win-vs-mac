#pragma once
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"

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
		 * @brief 角丸の半透明パネル（Fluentのサーフェス）を描画する
		 * @param x パネル左上のX座標
		 * @param y パネル左上のY座標
		 * @param width パネルの幅
		 * @param height パネルの高さ
		 */
		void drawPanel(int x, int y, int width, int height);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
	};
} // namespace game::ui::ingame
