#pragma once
#include <queue>
#include "Entity.h"

namespace core::ecs
{
	/**
	 * @brief EntityのIdの発行、回収を担当
	 */
	class EntityManager
	{
	public:
		static constexpr EntityId INITIAL_ENTITY_ID = 1;

		EntityManager() : m_nextId(INITIAL_ENTITY_ID){}
		
		/** @brief Entityを生成 */
		Entity create()
		{
			if (!m_recycledIds.empty())
			{
				EntityId id = m_recycledIds.front();
				m_recycledIds.pop();
				return Entity(id);
			}
			return Entity(m_nextId++);
		}

		/** @brief Entityを破棄 */
		void destroy(Entity entity)
		{
			m_recycledIds.push(entity.getId());
		}

	private:
		EntityId m_nextId;
		// 再利用可能なEntityIdのキュー
		std::queue<EntityId> m_recycledIds;
	};
}