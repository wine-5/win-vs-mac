#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/EntityManager.h"
#include "core/base/EventBus.h"
#include "core/interface/IRenderer.h"
#include "game/factory/EnemySpawner.h"
#include "game/event/InGameEvents.h"

namespace game::system
{
	/**
	 * @brief 敵の死亡後、赤化＋ディゾルブ演出を経てEntity・モデルハンドルを完全に後始末するSystem
	 *
	 * EnemyDeadEvent受信時にDeathComponentを付与してタイマー計測を開始し、
	 * 毎フレーム進行度に応じた赤化＋ディゾルブ（アルファフェード）をモデルに適用する。
	 * タイマーが尽きたら以下を行う：
	 * ・見た目をresetModelAppearanceで元に戻す
	 * ・EnemySpawner経由でモデルハンドルをアニメーションデタッチ済みの状態でプールへ返却
	 * ・EnemyFactory内部の追跡リストから除去
	 * ・Component全削除、Entity破棄（IDを再利用可能にする）
	 *
	 * これにより、死亡後もコライダー・攻撃対象として残り続ける問題を解消する
	 */
	class EnemyDeathSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief コンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param entityManager EntityManagerの参照（破棄に使用）
		 * @param eventBus EnemyDeadEvent購読用のEventBus
		 * @param enemySpawner モデルハンドル返却・EnemyFactory除去を委譲するEnemySpawner
		 * @param renderer 赤化＋ディゾルブ演出の適用・リセットに使うIRenderer
		 */
		EnemyDeathSystem(core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityManager& entityManager,
		    core::base::EventBus& eventBus,
		    game::factory::EnemySpawner& enemySpawner,
		    core::iface::IRenderer& renderer);

		/**
		 * @brief 死亡タイマーを更新し、尽きたEntityを後始末する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		void onEnemyDead(const event::EnemyDeadEvent& e);

		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityManager& m_entityManager;
		core::base::EventBus& m_eventBus;
		game::factory::EnemySpawner& m_enemySpawner;
		core::iface::IRenderer& m_renderer;
	};
} // namespace game::system
