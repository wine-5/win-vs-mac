#pragma once
#include "core/ecs/EntityManager.h"
#include "core/ecs/ComponentManager.h"
#include "core/utility/Vector3.h"
#include "game/stage/StageProp.h"
#include <memory>
#include <vector>

namespace game::factory
{
	/**
	 * @brief StageProp（床・壁などの配置物）の生成と寿命管理を担当
	 */
	class StagePropFactory
	{
	  public:
		/**
		 * @brief StagePropFactoryのコンストラクタ
		 * @param entityManager EntityManagerの参照
		 * @param componentManager ComponentManagerの参照
		 */
		StagePropFactory(
		    core::ecs::EntityManager& entityManager,
		    core::ecs::ComponentManager& componentManager);

		/**
		 * @brief 配置物を1つ生成する
		 * @param params 生成に必要な値一式
		 * @return 生成した配置物のEntityId
		 */
		core::ecs::EntityId create(const stage::StagePropParams& params);

	  private:
		core::ecs::EntityManager& m_entityManager;
		core::ecs::ComponentManager& m_componentManager;

		std::vector<std::unique_ptr<stage::StageProp>> m_props;
	};
} // namespace game::factory
