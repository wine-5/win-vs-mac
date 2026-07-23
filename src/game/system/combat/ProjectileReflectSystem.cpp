#include "ProjectileReflectSystem.h"
#include "game/component/ProjectileComponent.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/TagComponent.h"
#include "game/constant/Tag.h"
#include <cmath>
#include <vector>

namespace game::system::combat
{
	ProjectileReflectSystem::ProjectileReflectSystem(core::ecs::ComponentManager& componentManager)
	    : m_componentManager{ componentManager }
	{
	}

	void ProjectileReflectSystem::update(float /*deltaTime*/)
	{
		// 投擲物を陣営ごとに仕分ける（反射でTagを書き換えるため、先にIDを確定させておく）
		std::vector<core::ecs::EntityId> enemyProjectiles{};
		std::vector<core::ecs::EntityId> playerProjectiles{};

		for (auto id : m_componentManager.getAllEntities<component::ProjectileComponent>())
		{
			if (!m_componentManager.has<component::TagComponent>(id))
				continue;
			const auto tag{ m_componentManager.get<component::TagComponent>(id).m_tag };
			if (tag == constant::Tag::Enemy)
				enemyProjectiles.push_back(id);
			else if (tag == constant::Tag::Player)
				playerProjectiles.push_back(id);
		}

		if (enemyProjectiles.empty() || playerProjectiles.empty())
			return;

		for (auto enemyId : enemyProjectiles)
		{
			const auto& enemyTransform{ m_componentManager.get<component::TransformComponent>(enemyId) };
			// 接触半径は当たり判定（AttackComponent.m_attackRange）を流用する
			const float enemyRadius{ m_componentManager.has<component::AttackComponent>(enemyId)
				                         ? m_componentManager.get<component::AttackComponent>(enemyId).m_attackRange
				                         : 0.0f };

			for (auto playerId : playerProjectiles)
			{
				const auto& playerTransform{ m_componentManager.get<component::TransformComponent>(playerId) };
				const float playerRadius{ m_componentManager.has<component::AttackComponent>(playerId)
					                          ? m_componentManager.get<component::AttackComponent>(playerId).m_attackRange
					                          : 0.0f };

				const float dx{ enemyTransform.m_position.x - playerTransform.m_position.x };
				const float dy{ enemyTransform.m_position.y - playerTransform.m_position.y };
				const float dz{ enemyTransform.m_position.z - playerTransform.m_position.z };
				const float distanceSq{ dx * dx + dy * dy + dz * dz };
				const float threshold{ enemyRadius + playerRadius };

				if (distanceSq > threshold * threshold)
					continue;

				// 跳ね返し：速度を反転して逆方向へ、陣営をPlayerへ変更して敵に当たるようにする
				if (m_componentManager.has<component::VelocityComponent>(enemyId))
				{
					auto& velocity{ m_componentManager.get<component::VelocityComponent>(enemyId) };
					velocity.m_velocity.x = -velocity.m_velocity.x;
					velocity.m_velocity.y = -velocity.m_velocity.y;
					velocity.m_velocity.z = -velocity.m_velocity.z;
				}
				m_componentManager.get<component::TagComponent>(enemyId).m_tag = constant::Tag::Player;

				// 1発のプレイヤーWindow弾は複数の敵弾を弾ける（シールド的挙動）ので、
				// ここではプレイヤー弾は消さず、敵弾側だけ反射して次の敵弾へ進む
				break;
			}
		}
	}
} // namespace game::system::combat
