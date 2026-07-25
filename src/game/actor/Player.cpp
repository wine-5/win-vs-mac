#include "Player.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/movement/FallRecoveryComponent.h"
#include "game/component/visual/RenderComponent.h"
#include "game/component/visual/AnimationComponent.h"
#include "game/component/visual/WeaponAttachComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/TagComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/visual/HitEffectComponent.h"
#include "game/component/visual/EffectComponent.h"
#include "game/component/combat/PlayerChargeComponent.h"
#include "game/component/camera/CameraComponent.h"
#include "game/component/visual/LightComponent.h"
#include "game/component/camera/CameraEffectComponent.h"
#include "game/component/combat/AimComponent.h"
#include "game/constant/Tag.h"
#include "game/constant/AnimationId.h"
#include "game/constant/ModelId.h"

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
		// 床の縁から落ちても詰まないよう、直前に立っていた場所へ戻せるようにする
		componentManager.add<component::movement::FallRecoveryComponent>(m_entity.getId(), {});

		// アニメーションクリップの登録（状態→クリップの対応表）
		using constant::AnimationState;
		namespace anim_id = constant::animation_id;
		namespace priority = constant::animation_priority;
		constexpr float WALK_ANIM_SPEED{ 0.8f }; // 歩行アニメの再生速度（見た目の調整値）
		constexpr float RUN_ANIM_SPEED{ 1.0f };  // 走りアニメの再生速度（見た目の調整値）

		component::visual::AnimationComponent anim{};
		anim.m_clips[AnimationState::Idle]    = { resourceManager.loadAnimationById(anim_id::PLAYER_IDLE),  true };
		anim.m_clips[AnimationState::Walk] = { resourceManager.loadAnimationById(anim_id::PLAYER_WALK), true, AnimationState::Idle, priority::LOCOMOTION, WALK_ANIM_SPEED };
		anim.m_clips[AnimationState::Run]     = { resourceManager.loadAnimationById(anim_id::PLAYER_RUN),   true, AnimationState::Idle, priority::LOCOMOTION, RUN_ANIM_SPEED };
		anim.m_clips[AnimationState::Attack1] = { resourceManager.loadAnimationById(anim_id::PLAYER_SLASH), false, AnimationState::Idle,  priority::ATTACK };
		anim.m_clips[AnimationState::Attack2] = { resourceManager.loadAnimationById(anim_id::PLAYER_SPIN),  false, AnimationState::Idle,  priority::ATTACK };
		anim.m_clips[AnimationState::Hit]     = { resourceManager.loadAnimationById(anim_id::PLAYER_HIT),   false, AnimationState::Idle,  priority::HIT };
		anim.m_clips[AnimationState::Dying]   = { resourceManager.loadAnimationById(anim_id::PLAYER_DYING), false, AnimationState::Dying, priority::DYING };
		constexpr float JUMP_ANIM_START{ 30.0f }; // 頭の溜め約1.0秒（30fps×1.0）をカットして違和感を消す
		anim.m_clips[AnimationState::Jump] = { resourceManager.loadAnimationById(anim_id::PLAYER_JUMP), false, AnimationState::Idle, priority::JUMP, 1.0f, JUMP_ANIM_START };
		componentManager.add<component::visual::AnimationComponent>(m_entity.getId(), anim);
		componentManager.add<component::visual::RenderComponent>(m_entity.getId(), { modelHandle });
		attachWeapon(componentManager, resourceManager);
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
		// プレイヤーに追従する点光源。虚無の中で自機が沈まないようにしつつ、
		// 「自機が周囲を照らす」演出も兼ねる。頭上に置いて上から当てる
		component::visual::LightComponent light{};
		light.m_offset = core::Vector3{ 0.0f, 250.0f, 0.0f };
		light.m_range = 1200.0f;
		light.m_r = 220;
		light.m_g = 235;
		light.m_b = 255;
		componentManager.add<component::visual::LightComponent>(m_entity.getId(), light);

		componentManager.add<component::camera::CameraComponent>(m_entity.getId(), {});
		componentManager.add<component::camera::CameraEffectComponent>(m_entity.getId(), {});
		componentManager.add<component::combat::AimComponent>(m_entity.getId(), {});

		component::TagComponent tag{};
		tag.m_tag = constant::Tag::Player;
		componentManager.add<component::TagComponent>(m_entity.getId(), tag);
	}

	void Player::attachWeapon(core::ecs::ComponentManager& componentManager,
	    core::iface::IResourceManager& resourceManager)
	{
		// 剣モデルは制作中のため、暫定でボスの虹色くるくるを仮の剣として使う。
		// 装着位置・向き・大きさの調整はモデル差し替え後に実機を見ながら行う
		const int sourceHandle{ resourceManager.loadModelById(constant::model_id::MAC_RAINBOW_WHEEL) };
		if (sourceHandle == -1)
			return;

		// 装着描画は行列を直接指定するため、他の描き方と共有すると位置指定が
		// 効かなくなる。武器専用のハンドルを複製して持たせる
		const int weaponHandle{ resourceManager.duplicateModel(sourceHandle) };
		if (weaponHandle == -1)
			return;

		// 右手のボーン名。リグ依存のため、実際の名前が違えば起動時のログに
		// 候補一覧が出るので、それを見てここを直す
		constexpr std::string_view RIGHT_HAND_FRAME{ "mixamorig:RightHand" };

		component::visual::WeaponAttachComponent weapon{};
		weapon.m_modelHandle = weaponHandle;
		weapon.m_frameName = RIGHT_HAND_FRAME;
		componentManager.add<component::visual::WeaponAttachComponent>(m_entity.getId(), weapon);
	}

	core::ecs::EntityId Player::getId() const noexcept
	{
		return m_entity.getId();
	}
} // namespace game::actor