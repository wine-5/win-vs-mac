#include "RangedAttackSystem.h"
#include "game/component/InputComponent.h"
#include "game/component/CameraComponent.h"
#include "game/component/TransformComponent.h"
#include "game/constant/Tag.h"

namespace
{
	// TODO: 個別のjsonを作るタイミングでjsonへ移植する
	constexpr float PROJECTILE_SPEED{ 1500.0f }; // 弾速
	constexpr float PROJECTILE_DAMAGE{ 20.0f };  // ダメージ
	constexpr float PROJECTILE_LIFETIME{ 3.0f }; // 寿命（秒）
	constexpr float PROJECTILE_RADIUS{ 40.0f };  // 接触半径（当たり判定）
	constexpr float PROJECTILE_SCALE{ 1.0f };    // 見た目スケール
	constexpr float SPAWN_FORWARD{ 80.0f };      // プレイヤーより前方に出す距離
	constexpr float EYE_HEIGHT{ 100.0f };        // 発射源の高さ
	constexpr float FIRE_COOLDOWN{ 0.4f };       // 連射間隔（秒）
} // namespace

namespace game::system
{
	RangedAttackSystem::RangedAttackSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId,
	    factory::ProjectileFactory& projectileFactory)
	    : m_componentManager{ componentManager }, m_playerId{ playerId }, m_projectileFactory{ projectileFactory }
	{
	}

	void RangedAttackSystem::update(float deltaTime)
	{
		if (m_cooldownTimer > 0.0f)
			m_cooldownTimer -= deltaTime;

		if (!m_componentManager.has<component::InputComponent>(m_playerId) ||
		    !m_componentManager.has<component::CameraComponent>(m_playerId))
			return;

		const auto& input{ m_componentManager.get<component::InputComponent>(m_playerId) };
		if (!input.m_rangedAttackPressed || m_cooldownTimer > 0.0f)
			return;

		const auto& camera{ m_componentManager.get<component::CameraComponent>(m_playerId) };
		const auto& transform{ m_componentManager.get<component::TransformComponent>(m_playerId) };

		// カメラ前方へ、プレイヤーの少し前・目線の高さから発射する
		const core::Vector3 direction{ camera.m_forward };
		const core::Vector3 origin{
			transform.m_position.x + direction.x * SPAWN_FORWARD,
			transform.m_position.y + EYE_HEIGHT + direction.y * SPAWN_FORWARD,
			transform.m_position.z + direction.z * SPAWN_FORWARD
		};

		factory::ProjectileConfig config{};
		config.m_speed = PROJECTILE_SPEED;
		config.m_damage = PROJECTILE_DAMAGE;
		config.m_lifetime = PROJECTILE_LIFETIME;
		config.m_radius = PROJECTILE_RADIUS;
		config.m_scale = PROJECTILE_SCALE;

		m_projectileFactory.spawn(origin, direction, config, constant::Tag::Player);
		m_cooldownTimer = FIRE_COOLDOWN;
	}
} // namespace game::system
