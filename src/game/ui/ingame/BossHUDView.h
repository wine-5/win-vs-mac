#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "HudPanel.h"

namespace game::ui::ingame
{
	/**
	 * @brief インゲーム上中央のボスHPを描画するView
	 *
	 * ボスが出現している間だけ表示する。残りHPに加えて現在フェーズも出し、
	 * 「まだ終わらない」「性質が変わった」ことが一目で分かるようにする。
	 *
	 * レイアウトの数値はすべて1080p基準で持ち、画面高さに応じて拡大縮小する
	 */
	class BossHUDView
	{
	  public:
		/**
		 * @brief BossHUDViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager HealthComponent・MacAIComponentの読み出しに使う参照
		 */
		BossHUDView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager);

		/**
		 * @brief ボスHPを描画する
		 *
		 * ボスが未出現（IDが無効）なら何も描かない
		 * @param bossId ボスのEntityID
		 */
		void draw(core::ecs::EntityId bossId);

	  private:
		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		/**
		 * @brief 現在フェーズを示すピル（角丸のラベル）を描画する
		 * @param rightX ピルの右端X座標
		 * @param y ピル上端のY座標
		 * @param bossId フェーズの読み出し元となるボスのEntityID
		 */
		void drawPhasePill(int rightX, int y, core::ecs::EntityId bossId);

		/**
		 * @brief ボスのHPバーを描画する
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
		HudPanel m_panel;
	};
} // namespace game::ui::ingame
