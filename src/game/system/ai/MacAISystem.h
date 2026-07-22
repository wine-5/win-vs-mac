#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/base/EventBus.h"
#include "core/data/ProjectileMetadata.h"
#include "core/data/MacMetadata.h"
#include "core/utility/Vector3.h"
#include "game/factory/ProjectileFactory.h"
#include "game/factory/EnemySpawner.h"
#include "game/component/ai/MacAIComponent.h"
#include "game/component/TransformComponent.h"
#include <random>

namespace game::system::ai
{
	/**
	 * @brief ボス（Mac）のFSM駆動AIシステム
	 *
	 * MacAIComponentを持つ敵を状態機械で駆動する。
	 * 近接（SwingAttack）・遠距離（レインボー扇状）・召喚を重み付き抽選で使い分け、
	 * HP比率でフェーズ移行（覚醒）する。フェーズ別パラメータは macData.json から読み込む。
	 */
	class MacAISystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus フェーズ移行イベント発行用のEventBus
		 * @param projectileFactory レインボー弾生成用のProjectileFactory
		 * @param enemySpawner 雑魚召喚用のEnemySpawner
		 * @param rainbowMeta レインボー弾のメタデータ
		 * @param rainbowModelHandle レインボーモデルのハンドル
		 * @param rainbowRadius レインボー弾の当たり判定半径
		 * @param rainbowCenter レインボーモデルのAABB中心（回転の原点ズレ補正用）
		 */
		MacAISystem(core::ecs::ComponentManager& componentManager,
		    core::base::EventBus& eventBus,
		    factory::ProjectileFactory& projectileFactory,
		    factory::EnemySpawner& enemySpawner,
		    core::data::ProjectileMetadata rainbowMeta,
		    int rainbowModelHandle,
		    float rainbowRadius,
		    core::Vector3 rainbowCenter);

		/**
		 * @brief FSMを1フレーム分更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		/**
		 * @brief 次に実行する技を重み付きで抽選する
		 * @param phase 現在フェーズのパラメータ
		 * @param distance プレイヤーとの距離（近接候補判定に使う）
		 * @param canSummon 召喚上限に余裕があるか
		 * @return 抽選された状態（Melee/Ranged/Summon）。候補が無ければChase
		 */
		component::ai::MacState chooseAction(const core::data::MacPhaseData& phase,
		    float distance, bool canSummon);

		/**
		 * @brief 近接攻撃を実行する（攻撃要求＋Attack1アニメ）
		 * @param entityId ボスのEntityId
		 */
		void performMelee(core::ecs::EntityId entityId);

		/**
		 * @brief レインボー弾を扇状に発射する（＋Attack2アニメ）
		 * @param entityId ボスのEntityId
		 * @param transform ボスのTransform
		 * @param dirToTarget プレイヤーへの方向（正規化済み）
		 * @param phase 現在フェーズのパラメータ
		 */
		void performRanged(core::ecs::EntityId entityId, const component::TransformComponent& transform,
		    const core::Vector3& dirToTarget, const core::data::MacPhaseData& phase);

		/**
		 * @brief 全方位レインボー・ノヴァを発射する（覚醒限定。360度へ均等発射＋Attack2アニメ）
		 * @param entityId ボスのEntityId
		 * @param transform ボスのTransform
		 * @param phase 現在フェーズのパラメータ
		 */
		void performNova(core::ecs::EntityId entityId, const component::TransformComponent& transform,
		    const core::data::MacPhaseData& phase);

		/**
		 * @brief レインボー弾1発分の共通生成パラメータを作る（扇・ノヴァで共有）
		 * @param phase 現在フェーズのパラメータ
		 * @return 弾の生成設定（発射方向・原点・startEffectは呼び出し側で設定）
		 */
		[[nodiscard]] factory::ProjectileConfig makeRainbowConfig(const core::data::MacPhaseData& phase) const;

		/**
		 * @brief 雑魚を召喚する（ボス周辺のランダム座標）
		 * @param transform ボスのTransform
		 * @param phase 現在フェーズのパラメータ
		 * @param summonSlots 召喚可能な残り数
		 */
		void performSummon(const component::TransformComponent& transform,
		    const core::data::MacPhaseData& phase, int summonSlots);

		/**
		 * @brief 現在生存している雑魚（ボス以外の敵）の数を数える
		 * @return 生存雑魚数
		 */
		int countAliveMinions();

		/**
		 * @brief 水平方向の移動だけ止める（Y＝重力ぶんの落下速度は残す）
		 *
		 * 待機・溜め・攻撃中に velocity を全ゼロにすると重力が効かず宙に浮くため、
		 * X/ZのみゼロにしてY（重力）を残し、ボスが地面に接地し続けるようにする。
		 * @param entityId 対象のEntityId
		 */
		void stopHorizontalVelocity(core::ecs::EntityId entityId);

		core::ecs::ComponentManager& m_componentManager;
		core::base::EventBus& m_eventBus;
		factory::ProjectileFactory& m_projectileFactory;
		factory::EnemySpawner& m_enemySpawner;

		core::data::ProjectileMetadata m_rainbowMeta{};
		int m_rainbowModelHandle{ -1 };
		float m_rainbowRadius{ 0.0f };
		core::Vector3 m_rainbowCenter{ 0.0f, 0.0f, 0.0f };

		std::mt19937 m_rng{ std::random_device{}() };
	};
} // namespace game::system::ai
