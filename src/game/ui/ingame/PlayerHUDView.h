#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"

namespace game::ui::ingame
{
	/**
	 * @brief インゲーム左下のプレイヤーステータス（HPバー）を描画するView
	 *
	 * デザインはWindows 11（Fluent）に準拠する。パネルは角丸8pxの半透明サーフェス、
	 * HPバーはWindows 11のプログレスバーと同じく角丸のピル形状・単色で描く。
	 * 画像は使わず、すべて UIRenderer のプリミティブで組む。
	 *
	 * レイアウトの数値はすべて1080p基準で持ち、画面高さに応じて拡大縮小する
	 */
	class PlayerHUDView
	{
	  public:
		/**
		 * @brief PlayerHUDViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager HealthComponentの読み出しに使うComponentManagerの参照
		 */
		PlayerHUDView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager);

		/**
		 * @brief プレイヤーステータスを描画する
		 * @param playerId HPの読み出し元となるプレイヤーのEntityID
		 */
		void draw(core::ecs::EntityId playerId);

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

		/**
		 * @brief HPバーを描画する
		 *
		 * Windows 11のプログレスバーに倣い、溝と塗りをどちらも角丸のピル形状で描く。
		 * 残量に応じて緑→黄→赤と色が変わる
		 * @param x バー左上のX座標
		 * @param y バー左上のY座標
		 * @param width バーの幅（満タン時の長さ）
		 * @param height バーの高さ
		 * @param ratio HPの残量比（0.0〜1.0）
		 */
		void drawHealthBar(int x, int y, int width, int height, float ratio);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::ComponentManager& m_componentManager;
	};
} // namespace game::ui::ingame
