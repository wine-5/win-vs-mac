#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/base/EventBus.h"
#include "core/constant/SeType.h"
#include "game/attack/IDamageHandler.h"

namespace game::component::combat
{
	struct AttackComponent;
}

namespace game::system::combat
{
	/**
	 * @brief AttackComponentを持つEntityの攻撃処理・ダメージ計算を行うSystem
	 */
	class AttackSystem : public core::ecs::ISystem
	{
	public:
		/**
		 * @brief AttackSystemのコンストラクタ
		 * @param componentManager ComponentManagerの参照
		 * @param eventBus EventBusの参照
		 * @param playerAttackSeType プレイヤーが攻撃するときに再生するSEの種類
		 */
		AttackSystem(core::ecs::ComponentManager& componentManager, core::base::EventBus& eventBus,
			core::constant::SeType playerAttackSeType = core::constant::SeType::None);

		
		/**
		 * @brief 攻撃処理・ダメージ計算を更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	private:
	  /**
	   * @brief 攻撃範囲内の対象にダメージを解決し、ヒット/死亡イベントを発行する
	   * @param attackerId 攻撃者のEntityId
	   * @param attack 攻撃者のAttackComponent
	   */
	  void resolveAttack(core::ecs::EntityId attackerId, component::combat::AttackComponent& attack);

	  core::ecs::ComponentManager& m_componentManager;
	  core::base::EventBus& m_eventBus;
	  core::constant::SeType m_playerAttackSeType{ core::constant::SeType::None };
	  std::unique_ptr<attack::IDamageHandler> m_damageChain;
	};
} // namespace game::system::combat