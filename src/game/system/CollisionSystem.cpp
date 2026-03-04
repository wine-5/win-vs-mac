#include "CollisionSystem.h"
#include "game/component/TransformComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/component/VelocityComponent.h"

namespace game::system
{
	CollisionSystem::CollisionSystem(core::ecs::ComponentManager& componentManager)
		: m_componentManager(componentManager)
	{
	}

	void CollisionSystem::addEntity(core::ecs::EntityId id)
	{
		m_entities.push_back(id);
	}

	void CollisionSystem::update(float deltaTime)
	{
		for (int i = 0; i < m_entities.size(); i++)
		{
			for (int j = i + 1; j < m_entities.size(); j++)
			{
				if (isColliding(m_entities[i], m_entities[j]))
					resolveCollision(m_entities[i], m_entities[j]);
			}
		}
	}

	bool CollisionSystem::isColliding(core::ecs::EntityId a, core::ecs::EntityId b) const
	{
		auto& transformA = m_componentManager.get<component::TransformComponent>(a);
		auto& transformB = m_componentManager.get<component::TransformComponent>(b);
		auto& colliderA = m_componentManager.get<component::ColliderComponent>(a);
		auto& colliderB = m_componentManager.get<component::ColliderComponent>(b);

		// 各軸の中心座標
		core::Vector3 centerA = transformA.m_position + colliderA.m_offset;
		core::Vector3 centerB = transformB.m_position + colliderB.m_offset;

		// 各軸の重なりをチェック
		bool overlapX = abs(centerA.x - centerB.x) < (colliderA.m_size.x + colliderB.m_size.x) / 2.0f;
		bool overlapY = abs(centerA.y - centerB.y) < (colliderA.m_size.y + colliderB.m_size.y) / 2.0f;
		bool overlapZ = abs(centerA.z - centerB.z) < (colliderA.m_size.z + colliderB.m_size.z) / 2.0f;

		return overlapX && overlapY && overlapZ;
	}

	void CollisionSystem::resolveCollision(core::ecs::EntityId a, core::ecs::EntityId b)
	{
		auto& colliderA = m_componentManager.get<component::ColliderComponent>(a);
		auto& colliderB = m_componentManager.get<component::ColliderComponent>(b);

		// TagがPlayerとGroundの組み合わせを特定する
		core::ecs::EntityId playerId = -1;
		core::ecs::EntityId groundId = -1;

		if (colliderA.m_tag == constant::CollisionTag::Player &&
			colliderB.m_tag == constant::CollisionTag::Ground)
		{
			playerId = a;
			groundId = b;
		}
		else if (colliderA.m_tag == constant::CollisionTag::Ground &&
			colliderB.m_tag == constant::CollisionTag::Player)
		{
			playerId = b;
			groundId = a;
		}

		// Player vs Ground の押し返し処理
		if (playerId != -1 && groundId != -1)
		{
			auto& playerTransform = m_componentManager.get<component::TransformComponent>(playerId);
			auto& groundTransform = m_componentManager.get<component::TransformComponent>(groundId);
			auto& playerCollider = m_componentManager.get<component::ColliderComponent>(playerId);
			auto& groundCollider = m_componentManager.get<component::ColliderComponent>(groundId);

			// Groundの上面のY座標
			float groundTopY = groundTransform.m_position.y + groundCollider.m_size.y / 2.0f;

			// PlayerをGroundの上に乗せる
			playerTransform.m_position.y = groundTopY + playerCollider.m_size.y / 2.0f;
		}
	}
}