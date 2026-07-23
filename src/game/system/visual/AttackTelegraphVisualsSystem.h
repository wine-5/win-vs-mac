#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/interface/IRenderer.h"

namespace game::system
{
	/**
	 * @brief 敵の溜め攻撃（GroundSlam等）の予兆を地面に表示するSystem
	 *
	 * AttackComponentがワインドアップ中（m_windupPending）の近接攻撃者について、
	 * 攻撃範囲を示す円を足元の地面に描く。溜めの進行に合わせて内側の円が
	 * 中心から外側へ満ちていき、満ちきった瞬間にダメージが発生する。
	 * これによりプレイヤーは危険範囲と着弾タイミングを事前に認識できる。
	 * 弾（ProjectileComponent持ち）は地面予兆を出さない。
	 * drawはInGameViewの描画フェーズ（3D描画中）から呼ばれる。
	 */
	class AttackTelegraphVisualsSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief AttackTelegraphVisualsSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param renderer 地面への円描画に使うIRenderer
		 */
		AttackTelegraphVisualsSystem(core::ecs::ComponentManager& componentManager, core::iface::IRenderer& renderer);

		/**
		 * @brief 更新処理（状態はAttackComponentのワインドアップから読むため何もしない）
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

		/**
		 * @brief ワインドアップ中の近接攻撃者の足元に予兆円を描く（描画フェーズから呼ぶ）
		 */
		void draw();

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
	};
} // namespace game::system
