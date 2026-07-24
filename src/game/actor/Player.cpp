#include "Player.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/visual/RenderComponent.h"
#include "game/component/visual/AnimationComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/visual/HitEffectComponent.h"
#include "game/component/visual/EffectComponent.h"
#include "game/component/combat/PlayerChargeComponent.h"
#include "game/component/camera/CameraComponent.h"
#include "game/component/camera/CameraEffectComponent.h"
#include "game/component/combat/AimComponent.h"
#include "game/constant/Tag.h"
#include "game/constant/AnimationId.h"

namespace game::actor
{
	Player::Player(core::ecs::EntityManager& entityManager,
		core::ecs::ComponentManager& componentManager,
		core::iface::IResourceManager& resourceManager,
		int modelHandle,
		const data::PlayerData& playerData)
		: m_entity{entityManager.create()}
	{
		component::movement::TransformComponent transform{};
		transform.m_scale = playerData.getScale();
		componentManager.add<component::movement::TransformComponent>(m_entity.getId(), transform);
		componentManager.add<component::movement::VelocityComponent>(m_entity.getId(), {});
		componentManager.add<component::movement::InputComponent>(m_entity.getId(), {});

		// アニメーションクリップの登録（状態→クリップの対応表）
		using constant::AnimationState;
		namespace anim_id = constant::animation_id;
		namespace priority = constant::animation_priority;
		constexpr float WALK_ANIM_SPEED{ 0.6f }; // 歩行アニメの再生速度（見た目の調整値）
		constexpr float RUN_ANIM_SPEED{ 0.3f };  // 走りアニメの再生速度（見た目の調整値）

		component::visual::AnimationComponent anim{};
		anim.m_clips[AnimationState::Idle]    = { resourceManager.loadAnimationById(anim_id::PLAYER_IDLE),  true };
		anim.m_clips[AnimationState::Walk] = { resourceManager.loadAnimationById(anim_id::PLAYER_WALK), true, AnimationState::Idle, priority::LOCOMOTION, WALK_ANIM_SPEED };
		anim.m_clips[AnimationState::Run]     = { resourceManager.loadAnimationById(anim_id::PLAYER_RUN),   true, AnimationState::Idle, priority::LOCOMOTION, RUN_ANIM_SPEED };
		anim.m_clips[AnimationState::Attack1] = { resourceManager.loadAnimationById(anim_id::PLAYER_SLASH), false, AnimationState::Idle,  priority::ATTACK };
		anim.m_clips[AnimationState::Attack2] = { resourceManager.loadAnimationById(anim_id::PLAYER_SPIN),  false, AnimationState::Idle,  priority::ATTACK };
		anim.m_clips[AnimationState::Hit]     = { resourceManager.loadAnimationById(anim_id::PLAYER_HIT),   false, AnimationState::Idle,  priority::HIT };
		anim.m_clips[AnimationState::Dying]   = { resourceManager.loadAnimationById(anim_id::PLAYER_DYING), false, AnimationState::Dying, priority::DYING };
		anim.m_clips[AnimationState::Jump]    = { resourceManager.loadAnimationById(anim_id::PLAYER_JUMP),  false, AnimationState::Idle,  priority::JUMP };
		componentManager.add<component::visual::AnimationComponent>(m_entity.getId(), anim);
		componentManager.add<component::visual::RenderComponent>(m_entity.getId(), { modelHandle });
		componentManager.add<component::visual::HitEffectComponent>(m_entity.getId(), {});
		componentManager.add<component::visual::EffectComponent>(m_entity.getId(), {});

		component::combat::HealthComponent health{};
		health.m_maxHp = playerData.getMaxHp();
		health.m_currentHp = playerData.getMaxHp();
		health.m_defence = playerData.getDefence();
		componentManager.add<component::combat::HealthComponent>(m_entity.getId(), health);

		component::combat::AttackComponent attack{};
		attack.m_attackPower = playerData.getAttackPower();
		attack.m_attackRange = playerData.getAttackRange();
		attack.m_attackCooldown = playerData.getAttackCooldown();
		componentManager.add<component::combat::AttackComponent>(m_entity.getId(), attack);
		component::combat::ColliderComponent collider;
		collider.m_size = playerData.getColliderSize();
		collider.m_offset = playerData.getColliderOffset();
		componentManager.add<component::combat::ColliderComponent>(m_entity.getId(), collider);

		componentManager.add<component::combat::PlayerChargeComponent>(m_entity.getId(), {});
		componentManager.add<component::camera::CameraComponent>(m_entity.getId(), {});
		componentManager.add<component::camera::CameraEffectComponent>(m_entity.getId(), {});
		componentManager.add<component::combat::AimComponent>(m_entity.getId(), {});

		component::TagComponent tag{};
		tag.m_tag = constant::Tag::Player;
		componentManager.add<component::TagComponent>(m_entity.getId(), tag);
	}

	core::ecs::EntityId Player::getId() const noexcept
	{
		return m_entity.getId();
	}
} // namespace game::actor