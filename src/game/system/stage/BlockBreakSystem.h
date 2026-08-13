#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/EntityManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IResourceManager.h"
#include "core/base/EventBus.h"
#include "core/interface/IRenderer.h"
#include "game/factory/EnemySpawner.h"

namespace game::system::stage
{
	/**
	 * @brief プレイヤーの近接攻撃で壊せるブロックを壊すSystem
	 *
	 * ブロックはHPではなく打撃回数で壊れるため、AttackSystemのダメージ計算には乗せない。
	 * 代わりに「このフレームで当たり判定が解決されたか」（AttackComponent.m_justResolved）
	 * だけを見て、範囲内のブロックの打撃回数を1つ進め、ひびを進行させる。
	 * 規定回数に達したら破壊する。
	 *
	 * 遅延の秒数はここでは持たない。プレイヤーの攻撃には playerData.json の
	 * attackWindup ぶんの溜めがあり、AttackSystemがその時間を消化してから判定を解決する。
	 * 同じ秒数をこちら側でも持つと、attackWindup を調整したときに剣とブロックで
	 * タイミングがずれるため、解決された瞬間そのものに乗る。
	 *
	 * 弾では壊せない。遠距離で壊せてしまうと安全な位置から撃つだけになり、
	 * 近づいて剣を振る理由が消えるため。
	 *
	 * @note AttackSystemより後に登録すること。m_justResolved はAttackSystemが毎フレーム
	 *       立て直すため、前に置くと1フレーム古い状態を見ることになる
	 */
	class BlockBreakSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief BlockBreakSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param entityManager 拡張子の欠片を生成するためのEntityManager
		 * @param renderer ひびテクスチャの差し替えに使う描画インターフェース
		 * @param resourceManager 欠片のアイコン画像を読むリソース管理インターフェース
		 * @param eventBus 打撃・破壊を知らせるためのEventBus
		 * @param enemySpawner ギャンブルボックスを壊したときに敵を出すスポナー
		 * @param playerId プレイヤーのEntityID
		 */
		BlockBreakSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityManager& entityManager,
		    core::iface::IRenderer& renderer,
		    core::iface::IResourceManager& resourceManager,
		    core::base::EventBus& eventBus,
		    factory::EnemySpawner& enemySpawner,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 攻撃が成立していれば範囲内のブロックを壊しにかかる
		 * @param deltaTime フレーム間の時間差（未使用）
		 */
		void update(float deltaTime) override;

	  private:
		void hitBlock(core::ecs::EntityId blockId);
		void breakBlock(core::ecs::EntityId blockId);
		void spawnDrops(core::ecs::EntityId blockId);

		/**
		 * @brief 壊したブロックが枠を増やすものなら、装備できる個数を1つ増やす
		 *
		 * 欠片のドロップとは別の報酬。増えた枠は EquipSlotGainedEvent で知らせる
		 * @param blockId 壊れたブロックのEntityId
		 */
		void grantEquipSlot(core::ecs::EntityId blockId);

		/**
		 * @brief ギャンブルボックスの中身を決める（当たりか、敵か）
		 *
		 * 当たりと敵は排他。当たりを先に引き、外れたぶんが敵になる。
		 * 両方が同時に起きると「敵は出たが報酬ももらえた」になり、賭けが成立しない
		 * @param blockId 壊れたブロックのEntityId
		 */
		void resolveGamble(core::ecs::EntityId blockId);

		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityManager& m_entityManager;
		core::iface::IRenderer& m_renderer;
		core::iface::IResourceManager& m_resourceManager;
		core::base::EventBus& m_eventBus;
		factory::EnemySpawner& m_enemySpawner;
		core::ecs::EntityId m_playerId;
	};
} // namespace game::system::stage
