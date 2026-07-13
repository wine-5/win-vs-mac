#include "Player.h"
#include "game/component/TransformComponent.h"
#include "game/component/VelocityComponent.h"
#include "game/component/InputComponent.h"
#include "game/component/RenderComponent.h"
#include "game/component/AnimationComponent.h"
#include "game/component/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/HealthComponent.h"
#include "game/component/AttackComponent.h"
#include "game/component/HitEffectComponent.h"
#include "game/component/EffectComponent.h"
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
		component::TransformComponent transform{};
		transform.m_scale = playerData.getScale();
		componentManager.add<component::TransformComponent>(m_entity.getId(), transform);
		componentManager.add<component::VelocityComponent>(m_entity.getId(), {});
		componentManager.add<component::InputComponent>(m_entity.getId(), {});

		// アニメーションクリップの登録（状態→クリップの対応表）
		using constant::AnimationState;
		namespace anim_id = constant::animation_id;
		component::AnimationComponent anim{};
		anim.m_clips[AnimationState::Idle]    = { resourceManager.loadAnimationById(anim_id::PLAYER_IDLE),  true };
		anim.m_clips[AnimationState::Walk]    = { resourceManager.loadAnimationById(anim_id::PLAYER_WALK),  true, AnimationState::Idle, 0, 0.6f };
		anim.m_clips[AnimationState::Run]     = { resourceManager.loadAnimationById(anim_id::PLAYER_RUN),   true };
		anim.m_clips[AnimationState::Attack1] = { resourceManager.loadAnimationById(anim_id::PLAYER_SLASH), false, AnimationState::Idle,  30 };
		anim.m_clips[AnimationState::Attack2] = { resourceManager.loadAnimationById(anim_id::PLAYER_SPIN),  false, AnimationState::Idle,  30 };
		anim.m_clips[AnimationState::Hit]     = { resourceManager.loadAnimationById(anim_id::PLAYER_HIT),   false, AnimationState::Idle,  50 };
		anim.m_clips[AnimationState::Dying]   = { resourceManager.loadAnimationById(anim_id::PLAYER_DYING), false, AnimationState::Dying, 100 };
		anim.m_clips[AnimationState::Jump]    = { resourceManager.loadAnimationById(anim_id::PLAYER_JUMP),  false, AnimationState::Idle,  20 };
		componentManager.add<component::AnimationComponent>(m_entity.getId(), anim);
		componentManager.add<component::RenderComponent>(m_entity.getId(), { modelHandle });
		componentManager.add<component::HitEffectComponent>(m_entity.getId(), {});
		componentManager.add<component::EffectComponent>(m_entity.getId(), {});

		component::HealthComponent health{};
		health.m_maxHp = playerData.getMaxHp();
		health.m_currentHp = playerData.getMaxHp();
		health.m_defence = playerData.getDefence();
		componentManager.add<component::HealthComponent>(m_entity.getId(), health);

		component::AttackComponent attack{};
		attack.m_attackPower = playerData.getAttackPower();
		attack.m_attackRange = playerData.getAttackRange();
		attack.m_attackCooldown = playerData.getAttackCooldown();
		componentManager.add<component::AttackComponent>(m_entity.getId(), attack);
		component::ColliderComponent collider;
		collider.m_size = playerData.getColliderSize();
		collider.m_offset = playerData.getColliderOffset();
		componentManager.add<component::ColliderComponent>(m_entity.getId(), collider);

		component::TagComponent tag{};
		tag.m_tag = constant::Tag::Player;
		componentManager.add<component::TagComponent>(m_entity.getId(), tag);
	}

	core::ecs::EntityId Player::getId() const noexcept
	{
		return m_entity.getId();
	}
}