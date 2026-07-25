#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/base/EventBus.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include "core/utility/Vector3.h"
#include "game/event/InGameEvents.h"
#include <vector>

namespace game::system::visual
{
	/**
	 * @brief 敵に与えたダメージ量を被弾位置に表示し、浮かせながら消すSystem
	 *
	 * AttackHitEvent を購読し、敵の頭上にダメージ数値を出す。
	 * 数値は発生位置をワールド座標で覚えたままその場から浮き上がるので、
	 * 敵が動いても「どこに当てたか」が残る。
	 *
	 * 表示物はEntityを持たず本Systemが直接保持する。ダメージ数値は
	 * 当たり判定も参照関係も持たない短命な見た目だけの存在で、
	 * Entityにすると生成・破棄の負荷とIDの消費が見合わないため。
	 */
	class DamagePopupSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief DamagePopupSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus AttackHitEvent購読用のEventBus
		 * @param renderer ワールド座標→スクリーン座標の変換に使うIRenderer
		 * @param uiRenderer 数値描画に使うIUIRenderer
		 * @param screen 画面サイズ取得のインターフェース（文字サイズの解像度非依存化）
		 */
		DamagePopupSystem(core::ecs::ComponentManager& componentManager,
		    core::base::EventBus& eventBus,
		    core::iface::IRenderer& renderer,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen);

		/**
		 * @brief 表示中の数値の経過時間を進め、寿命が尽きたものを取り除く
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief 表示中のダメージ数値を描画する（InGameViewの描画フェーズから呼ぶ）
		 */
		void draw();

	  private:
		/** @brief 表示中のダメージ数値1つぶん */
		struct Popup
		{
			core::Vector3 m_worldPosition{}; // 発生位置（ワールド座標。ここから浮き上がる）
			int m_damage{ 0 };               // 表示する数値
			float m_elapsedTime{ 0.0f };     // 表示開始からの経過時間（秒）
		};

		/**
		 * @brief 被弾イベントを受けて数値を積む
		 * @param event 被弾イベント
		 */
		void onAttackHit(const game::event::AttackHitEvent& event);

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;

		std::vector<Popup> m_popups{};

		// EventBusの購読ハンドル。このクラスが破棄されると自動で解除される
		std::vector<core::base::EventBus::Subscription> m_subscriptions{};
	};
} // namespace game::system::visual
