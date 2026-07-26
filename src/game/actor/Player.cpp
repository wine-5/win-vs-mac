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
#include "game/component/combat/AttackComboComponent.h"
#include "game/component/visual/HitEffectComponent.h"
#include "game/component/visual/EffectComponent.h"
#include "game/component/combat/PlayerChargeComponent.h"
#include "game/component/camera/CameraComponent.h"
#include "game/component/visual/LightComponent.h"
#include "game/component/camera/CameraEffectComponent.h"
#include "game/component/combat/AimComponent.h"
#include "game/actor/AnimationSetup.h"
#include "game/constant/Tag.h"
#include "game/constant/ModelId.h"
#include "core/utility/MathConstants.h"
#include <algorithm>

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

		// アニメーションクリップは playerData.json の animations 配列で定義する。
		// 再生速度・優先度・開始位置を再ビルドなしで調整できるようにするため、
		// 敵と同じデータ駆動の形式に揃えている
		componentManager.add<component::visual::AnimationComponent>(
		    m_entity.getId(), buildAnimationComponent(playerData.getAnimations(), resourceManager));
		componentManager.add<component::visual::RenderComponent>(m_entity.getId(), { modelHandle });
		attachWeapon(componentManager, resourceManager, playerData);
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
		// 振り下ろしの瞬間にダメージが出るよう、アニメーションの溜め分だけ判定を遅らせる
		attack.m_windupDelay = playerData.getAttackWindup();
		componentManager.add<component::combat::AttackComponent>(m_entity.getId(), attack);

		// 近接攻撃の2段コンボ（斬り→回転斬り）。次段の受付時間はJSONで調整する
		component::combat::AttackComboComponent combo{};
		combo.m_inputWindow = playerData.getComboInputWindow();
		combo.m_stage2DamageMultiplier = playerData.getComboStage2Multiplier();
		componentManager.add<component::combat::AttackComboComponent>(m_entity.getId(), combo);

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
	    core::iface::IResourceManager& resourceManager,
	    const data::PlayerData& playerData)
	{
		// weapon要素が無ければ武器を持たない（素手のキャラにも使えるようにする）
		const auto& config{ playerData.getWeapon() };
		if (!config.has_value())
			return;

		const int sourceHandle{ resourceManager.loadModelById(config->modelId) };
		if (sourceHandle == -1)
			return;

		// 装着描画は行列を直接指定するため、他の描き方と共有すると位置指定が
		// 効かなくなる。武器専用のハンドルを複製して持たせる
		const int weaponHandle{ resourceManager.duplicateModel(sourceHandle) };
		if (weaponHandle == -1)
			return;

		// 見た目の長さから必要な拡大率を逆算する。モデルの実寸は変換ツールの単位系
		// （メートルかセンチメートルか）で桁が変わるため、倍率を固定値で置くと
		// モデルを作り直すたびに破綻する。
		// なお手のボーン行列にはプレイヤーの拡大率が既に乗っているため、その分を割り戻す
		const float playerScale{ playerData.getScale().x };
		const core::Vector3 modelSize{ resourceManager.computeBoundingSize(weaponHandle) };
		const float modelLength{ std::max({ modelSize.x, modelSize.y, modelSize.z }) };

		float weaponScale{ 1.0f };
		if (config->length > 0.0f && modelLength > 0.0f && playerScale > 0.0f)
			weaponScale = config->length / (modelLength * playerScale);

		component::visual::WeaponAttachComponent weapon{};
		weapon.m_modelHandle = weaponHandle;
		weapon.m_frameName = config->frameName;
		weapon.m_offsetPosition = config->offsetPosition;
		// JSONは度で持つのでラジアンへ変換する（手で角度を書くときに度のほうが扱いやすい）
		weapon.m_offsetRotation = core::Vector3{
			config->offsetRotation.x * core::utility::DEG_TO_RAD,
			config->offsetRotation.y * core::utility::DEG_TO_RAD,
			config->offsetRotation.z * core::utility::DEG_TO_RAD
		};
		weapon.m_offsetScale = core::Vector3{ weaponScale, weaponScale, weaponScale };
		componentManager.add<component::visual::WeaponAttachComponent>(m_entity.getId(), weapon);
	}

	core::ecs::EntityId Player::getId() const noexcept
	{
		return m_entity.getId();
	}
} // namespace game::actor