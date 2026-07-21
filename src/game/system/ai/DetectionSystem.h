#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/base/EventBus.h"

namespace game::system::ai
{
	/**
	 * @brief 敵がプレイヤーを発見した瞬間を検知するSystem（全敵共通）
	 *
	 * 敵の種類（近接/遠距離/ボス）に依らず、AIComponentの追跡対象と索敵範囲だけを見て
	 * 「未索敵→索敵」に切り替わったフレームを検知し、EnemyAlertedEventを発行する。
	 * これにより発見演出（通知バッジなど）をXcode/Safari/Mac共通で使い回せる。
	 * 実際の演出は本Systemの責務外（DetectionAlertSystemが担当）。
	 */
	class DetectionSystem : public core::ecs::ISystem
	{
	  public:
		/**
		 * @brief DetectionSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus EnemyAlertedEvent発行用のEventBusの参照
		 */
		DetectionSystem(core::ecs::ComponentManager& componentManager, core::base::EventBus& eventBus);

		/**
		 * @brief 全敵の索敵状態を更新し、発見の瞬間にEnemyAlertedEventを発行する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	  private:
		core::ecs::ComponentManager& m_componentManager;
		core::base::EventBus& m_eventBus;
	};
} // namespace game::system::ai
