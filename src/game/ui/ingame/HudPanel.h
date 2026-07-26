#pragma once
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <chrono>

namespace game::ui::ingame
{
	/**
	 * @brief HUDの各パネルに共通する下地（Windows 11 / Fluentのサーフェス）を描画するクラス
	 *
	 * 角丸8pxの半透明な面と、その上に載る細い枠線だけを担当する。
	 * 中身（文字・バー・アイコン）は各Viewが自分で描く。
	 *
	 * レイアウトの数値はすべて1080p基準で持ち、画面高さに応じて拡大縮小する
	 */
	class HudPanel
	{
	  public:
		/**
		 * @brief HudPanelのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 */
		HudPanel(core::iface::IUIRenderer& uiRenderer, core::iface::IScreen& screen);

		/**
		 * @brief パネルの下地を描画する
		 * @param x パネル左上のX座標
		 * @param y パネル左上のY座標
		 * @param width パネルの幅
		 * @param height パネルの高さ
		 */
		void draw(int x, int y, int width, int height);

	  private:
		/**
		 * @brief 面の上を光の帯が左から右へ一度だけ横切る演出を描画する
		 *
		 * 一定間隔でだけ走らせる。常に光らせ続けると「異常が起きている」合図に見えてしまうため、
		 * 平常時の生存確認として、間を置いて短く光らせる
		 * @param x パネル左上のX座標
		 * @param y パネル左上のY座標
		 * @param width パネルの幅
		 * @param height パネルの高さ
		 */
		void drawSweep(int x, int y, int width, int height);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;

		// 光沢の基準時刻。描画経路からしか呼ばれずdeltaTimeを受け取らないため、
		// 経過時間は壁時計から求める
		std::chrono::steady_clock::time_point m_startTime{ std::chrono::steady_clock::now() };
	};
} // namespace game::ui::ingame
