#include "CollisionSystem.h"
#include "game/component/TransformComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/constant/Tag.h"
#include <cmath>

namespace game::system
{
	CollisionSystem::CollisionSystem(core::ecs::ComponentManager& componentManager)
		: m_componentManager{ componentManager }
	{
	}

	void CollisionSystem::update(float deltaTime)
	{
		auto entities{m_componentManager.getAllEntities<component::ColliderComponent>()};

		for (size_t i = 0; i < entities.size(); i++)
		{
			for (size_t j = i + 1; j < entities.size(); j++)
			{
				if (isColliding(entities[i], entities[j]))
					resolveCollision(entities[i], entities[j]);
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
		core::Vector3 centerA{transformA.m_position + colliderA.m_offset};
		core::Vector3 centerB{transformB.m_position + colliderB.m_offset};

		// 各軸の距離と必要な距離
		float distX{std::abs(centerA.x - centerB.x)};
		float distY{std::abs(centerA.y - centerB.y)};
		float distZ{std::abs(centerA.z - centerB.z)};

		float requiredX{(colliderA.m_size.x + colliderB.m_size.x) / 2.0f};
		float requiredY{(colliderA.m_size.y + colliderB.m_size.y) / 2.0f};
		float requiredZ{(colliderA.m_size.z + colliderB.m_size.z) / 2.0f};

		// 各軸の重なりをチェック
		bool overlapX{distX <= requiredX};
		bool overlapY{distY <= requiredY};
		bool overlapZ{distZ <= requiredZ};

		return overlapX && overlapY && overlapZ;
	}

	void CollisionSystem::resolveCollision(core::ecs::EntityId a, core::ecs::EntityId b)
	{
		auto& colliderA = m_componentManager.get<component::ColliderComponent>(a);
		auto& colliderB = m_componentManager.get<component::ColliderComponent>(b);

		// TagがPlayerとGroundの組み合わせを特定する
		core::ecs::EntityId playerId{core::ecs::INVALID_ENTITY_ID};
		core::ecs::EntityId groundId{core::ecs::INVALID_ENTITY_ID};

		auto& tagA{ m_componentManager.get<component::TagComponent>(a) };
		auto& tagB{ m_componentManager.get<component::TagComponent>(b) };

		if (tagA.m_tag == constant::Tag::Player && tagB.m_tag == constant::Tag::Ground)
		{
			playerId = a;
			groundId = b;
		}
		else if (tagA.m_tag == constant::Tag::Ground && tagB.m_tag == constant::Tag::Player)
		{
			playerId = b;
			groundId = a;
		}

		// Player vs Ground の押し返し処理
		if (playerId != core::ecs::INVALID_ENTITY_ID && groundId != core::ecs::INVALID_ENTITY_ID)
		{
			auto& playerTransform = m_componentManager.get<component::TransformComponent>(playerId);
			auto& groundTransform = m_componentManager.get<component::TransformComponent>(groundId);
			auto& playerCollider = m_componentManager.get<component::ColliderComponent>(playerId);
			auto& groundCollider = m_componentManager.get<component::ColliderComponent>(groundId);
			auto& playerVelocity = m_componentManager.get<component::VelocityComponent>(playerId);

			// 各コライダーの中心とAABBの境界を計算
			core::Vector3 playerCenter{playerTransform.m_position + playerCollider.m_offset};
			core::Vector3 groundCenter{groundTransform.m_position + groundCollider.m_offset};

			float playerBottom{playerCenter.y - playerCollider.m_size.y / 2.0f};
			float groundTop{groundCenter.y + groundCollider.m_size.y / 2.0f};

			// Playerが地面より下、または地面に近い場合に補正
			if (playerBottom <= groundTop)
			{
				// Playerの下端を地面の上端に合わせる
				float correction{groundTop - playerBottom};
				playerTransform.m_position.y += correction;

				// 下方向の速度をリセット（地面に着地）
				if (playerVelocity.m_velocity.y < 0.0f)
					playerVelocity.m_velocity.y = 0.0f;
			}


		}

		// Enemy vs Ground の組み合わせを特定する
		core::ecs::EntityId enemyId{ core::ecs::INVALID_ENTITY_ID };
		groundId = core::ecs::INVALID_ENTITY_ID;

		if (tagA.m_tag == constant::Tag::Enemy && tagB.m_tag == constant::Tag::Ground)
		{
			enemyId = a;
			groundId = b;
		}
		else if (tagA.m_tag == constant::Tag::Ground && tagB.m_tag == constant::Tag::Enemy)
		{
			enemyId = b;
			groundId = a;
		}

		// Enemy vs Ground の押し返し処理
		if (enemyId != core::ecs::INVALID_ENTITY_ID && groundId != core::ecs::INVALID_ENTITY_ID)
		{
			auto& enemyTransform = m_componentManager.get<component::TransformComponent>(enemyId);
			auto& groundTransform = m_componentManager.get<component::TransformComponent>(groundId);
			auto& enemyCollider = m_componentManager.get<component::ColliderComponent>(enemyId);
			auto& groundCollider = m_componentManager.get<component::ColliderComponent>(groundId);
			auto& enemyVelocity = m_componentManager.get<component::VelocityComponent>(enemyId);

			core::Vector3 enemyCenter{enemyTransform.m_position + enemyCollider.m_offset};
			core::Vector3 groundCenter{groundTransform.m_position + groundCollider.m_offset};

			float enemyBottom{enemyCenter.y - enemyCollider.m_size.y / 2.0f};
			float groundTop{groundCenter.y + groundCollider.m_size.y / 2.0f};

			if (enemyBottom <= groundTop)
			{
				float correction{groundTop - enemyBottom};
				enemyTransform.m_position.y += correction;

				if (enemyVelocity.m_velocity.y < 0.0f)
					enemyVelocity.m_velocity.y = 0.0f;
			}
		}
	}
} // namespace game::system