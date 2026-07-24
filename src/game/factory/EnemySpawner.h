#pragma once
#include "core/ecs/Entity.h"
#include "core/ecs/ComponentManager.h"
#include "core/interface/IResourceManager.h"
#include "core/utility/Vector3.h"
#include "core/base/EventBus.h"
#include "game/constant/EnemyType.h"
#include <unordered_map>
#include <vector>

namespace game::factory
{
	class FactoryManager;

	/**
	 * @brief 敵の生成に必要なゲーム文脈を組み立てる高レベルなスポナー（オーケストレーター）
	 *
	 * モデルロード〜複製ハンドル生成〜メタデータからのEnemyData構築〜
	 * EnemyFactoryへの生成依頼〜追跡対象(AIComponent)の注入までを一貫して行う。
	 * 「どんな敵を・どこに・どういう状態で置くか」を決めるのが責務であり、
	 * ResourceManager・ComponentManager・ステージ配置定義など複数サブシステムを束ねる。
	 * 初期スポーン（ステージ配置定義）とボスによる実行時召喚の両方から再利用できる。
	 *
	 * 実際のインスタンス化と寿命管理は EnemyFactory に委譲する。
	 * リソースロードやAI設定といったゲーム文脈をFactory側に持ち込ませず、
	 * Factoryを純粋な「型→オブジェクト」の生成抽象に保つために両者を分離している。
	 */
	class EnemySpawner
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param factoryManager FactoryManagerの参照
		 * @param componentManager ComponentManagerの参照
		 * @param resourceManager IResourceManagerの参照
		 * @param eventBus スポーン時にEnemySpawnedEventを発行するEventBusの参照
		 */
		EnemySpawner(
		    FactoryManager& factoryManager,
		    core::ecs::ComponentManager& componentManager,
		    core::iface::IResourceManager& resourceManager,
		    core::base::EventBus& eventBus);

		/**
		 * @brief 生成する敵の追跡対象（通常はプレイヤー）を設定する
		 * @param target 追跡対象のエンティティ
		 */
		void setTargetEntity(core::ecs::Entity target) noexcept;

		/**
		 * @brief 型と座標を指定して敵を1体生成する
		 * @param type 敵の種類
		 * @param position 生成位置
		 * @param rotationY 初期の向き（度数法）。ボスの召喚など向きを問わない場合は既定の0でよい
		 * @return 生成したEnemyのEntityId
		 */
		core::ecs::EntityId spawn(constant::EnemyType type, const core::Vector3& position,
		    float rotationY = 0.0f);

		/**
		 * @brief ステージ配置定義に基づき雑魚敵を全て生成する
		 *
		 */
		void spawnStageEnemies();

		/**
		 * @brief 死亡した敵の後始末をする（EnemyDeathSystemから呼ばれる）
		 *
		 * モデルハンドルをアニメーションデタッチ済みの状態で種別ごとのプールへ返却し、
		 * EnemyFactory内部の追跡リストからも除去する。
		 * Entity・Componentそのものの破棄は呼び出し側（EnemyDeathSystem）の責務
		 * @param type 敵の種類
		 * @param entityId 破棄対象のEntityId
		 * @param modelHandle 返却するモデルハンドル
		 */
		void returnEnemy(constant::EnemyType type, core::ecs::EntityId entityId, int modelHandle);

	  private:
		/**
		 * @brief 種別ごとのモデルハンドルプールから取得する。プールが空なら新規複製する
		 * @param type 敵の種類
		 * @param modelId モデルID（新規複製が必要な場合のロードに使う）
		 * @return モデルハンドル
		 */
		int acquireModelHandle(constant::EnemyType type, std::string_view modelId);

		FactoryManager& m_factoryManager;
		core::ecs::ComponentManager& m_componentManager;
		core::iface::IResourceManager& m_resourceManager;
		core::base::EventBus& m_eventBus;

		// 生成した敵に設定する追跡対象。未設定(0)なら設定しない
		core::ecs::Entity m_target{ 0 };

		// 種別ごとに使い回すモデルハンドルのプール（死亡した敵から返却されたもの）
		std::unordered_map<constant::EnemyType, std::vector<int>> m_modelHandlePool;
	};
} // namespace game::factory
