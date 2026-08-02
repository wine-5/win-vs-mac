#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IRenderer.h"

namespace game::system::stage
{
	/**
	 * @brief プレイヤーの近接攻撃で壊せるブロックを壊すSystem
	 *
	 * ブロックはHPではなく打撃回数で壊れるため、AttackSystemのダメージ計算には乗せない。
	 * 代わりに「このフレームで攻撃が成立したか」（AttackComponent.m_justFired）だけを
	 * 見て、範囲内のブロックの打撃回数を1つ進め、ひびを進行させる。
	 * 規定回数に達したら破壊する。
	 *
	 * 弾では壊せない。遠距離で壊せてしまうと安全な位置から撃つだけになり、
	 * 近づいて剣を振る理由が消えるため。
	 *
	 * @note AttackSystemより後に登録すること。m_justFired はAttackSystemが毎フレーム
	 *       立て直すため、前に置くと1フレーム古い状態を見ることになる
	 */
	class BlockBreakSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief BlockBreakSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param renderer ひびテクスチャの差し替えに使う描画インターフェース
		 * @param playerId プレイヤーのEntityID
		 */
		BlockBreakSystem(core::ecs::ComponentManager& componentManager,
		    core::iface::IRenderer& renderer,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 攻撃が成立していれば範囲内のブロックを壊しにかかる
		 * @param deltaTime フレーム間の時間差（未使用）
		 */
		void update(float deltaTime) override;

	  private:
		void hitBlock(core::ecs::EntityId blockId);

		core::ecs::ComponentManager& m_componentManager;
		core::iface::IRenderer& m_renderer;
		core::ecs::EntityId m_playerId;
	};
} // namespace game::system::stage
