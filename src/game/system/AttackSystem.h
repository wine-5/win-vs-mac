#pragma once
#include "core/ecs/ISystem.h"
#include "core/ecs/ComponentManager.h"
#include "core/EventBus.h"
#include "game/attack/IDamageHandler.h"

namespace game::system
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
		 */
		AttackSystem(core::ecs::ComponentManager& componentManager, EventBus& eventBus);

		
		/**
		 * @brief 攻撃処理・ダメージ計算を更新する
		 * @param deltaTime フレーム間の時間差
		 */
		void update(float deltaTime) override;

	private:
		core::ecs::ComponentManager& m_componentManager;
		EventBus& m_eventBus;
		std::unique_ptr<attack::IDamageHandler> m_damageChain;
	};
}