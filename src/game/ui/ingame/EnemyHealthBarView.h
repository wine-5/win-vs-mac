#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/interface/IRenderer.h"

namespace game::ui::ingame
{
	/**
	 * @brief 雑魚敵の頭上に残りHPを描画するView
	 *
	 * 一度でもダメージを与えた敵だけに表示する。全員に常時出すと画面が
	 * バーだらけになり、いま戦っている相手が逆に埋もれるため。
	 * ボスは上中央に専用のHUDがあるので対象外にする。
	 *
	 * レイアウトの数値はすべて1080p基準で持ち、画面高さに応じて拡大縮小する
	 */
	class EnemyHealthBarView
	{
	  public:
		/**
		 * @brief EnemyHealthBarViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager 敵の状態を読み出すComponentManagerの参照
		 * @param renderer ワールド座標をスクリーン座標へ変換するのに使う
		 */
		EnemyHealthBarView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager,
		    core::iface::IRenderer& renderer);

		/**
		 * @brief 敵の頭上HPバーを描画する
		 * @param bossId ボスのEntityID（専用HUDがあるため描画対象から外す）
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
		 * @brief 1体ぶんのバーを頭上に描画する
		 * @param entityId 対象の敵のEntityID
		 * @param ratio HPの残量比（0.0〜1.0）
		 */
		void drawBarAboveHead(core::ecs::EntityId entityId, float ratio);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
	};
} // namespace game::ui::ingame
