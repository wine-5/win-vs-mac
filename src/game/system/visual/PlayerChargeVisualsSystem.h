#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"

namespace game::system::visual
{
	/**
	 * @brief プレイヤーの溜め攻撃の画面演出（集中線）を担当するSystem
	 *
	 * PlayerChargeComponentの溜め状態を読み取り、画面端から中心へ向かう
	 * 漫画風のくさび形集中線を描画する（中央は空けて視界を確保する）。
	 * updateで演出用の時間（ちらつきアニメーション）を進め、
	 * drawはInGameViewの描画フェーズから呼ばれる（描画順はViewが管理し、描画内容は本Systemが持つ）。
	 */
	class PlayerChargeVisualsSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief PlayerChargeVisualsSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param playerId プレイヤーのEntityID
		 */
		PlayerChargeVisualsSystem(core::ecs::ComponentManager& componentManager,
		    core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 演出用の時間を進める（溜め中のみ回転アニメーションが進行する）
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief 溜め状態に応じて集中線を描画する（InGameViewの描画フェーズから呼ぶ）
		 */
		void draw();

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::EntityId m_playerId;

		float m_animationTime{ 0.0f }; // 溜め中に蓄積する演出用時間（集中線のちらつきに使う）
	};
} // namespace game::system::visual
