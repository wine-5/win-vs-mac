#include "PlayerAttackComboSystem.h"
#include "game/component/combat/AttackComboComponent.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/visual/AnimationComponent.h"
#include "game/constant/AnimationState.h"

namespace
{
	// コンボの最終段。ここまで進んだら次の入力は1段目へ戻る
	constexpr int MAX_COMBO_STAGE{ 2 };

	// 段ごとの斬撃エフェクトの傾き（ラジアン）。同じエフェクトを使い回すため、
	// 1段目は正面への振り下ろし、2段目は水平に寝かせて回転斬りに見せる。
	// 実機での見た目に合わせた調整値
	constexpr float STAGE2_EFFECT_ROLL{ 1.5708f }; // 90度

	const core::Vector3 STAGE1_EFFECT_ROTATION{ 0.0f, 0.0f, 0.0f };
	const core::Vector3 STAGE2_EFFECT_ROTATION{ 0.0f, 0.0f, STAGE2_EFFECT_ROLL };
} // namespace

namespace game::system::combat
{
	PlayerAttackComboSystem::PlayerAttackComboSystem(core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_playerId{ playerId }
	{
	}

	void PlayerAttackComboSystem::update(float deltaTime)
	{
		if (!m_componentManager.has<component::combat::AttackComboComponent>(m_playerId) ||
		    !m_componentManager.has<component::combat::AttackComponent>(m_playerId) ||
		    !m_componentManager.has<component::movement::InputComponent>(m_playerId) ||
		    !m_componentManager.has<component::visual::AnimationComponent>(m_playerId))
			return;

		auto& combo{ m_componentManager.get<component::combat::AttackComboComponent>(m_playerId) };
		auto& attack{ m_componentManager.get<component::combat::AttackComponent>(m_playerId) };
		const auto& input{ m_componentManager.get<component::movement::InputComponent>(m_playerId) };

		// 受付時間が切れたらコンボを最初から積み直す
		if (combo.m_windowTimer > 0.0f)
		{
			combo.m_windowTimer -= deltaTime;
			if (combo.m_windowTimer <= 0.0f)
				combo.m_stage = 0;
		}

		// 押した瞬間だけを拾う（押しっぱなしで段が進み続けるのを防ぐ）
		const bool isPressedNow{ input.m_attackPressed };
		const bool isJustPressed{ isPressedNow && !m_wasAttackPressed };
		m_wasAttackPressed = isPressedNow;

		if (!isJustPressed)
			return;

		// 前の攻撃がまだ終わっていない間は受け付けない（連打で段が飛ぶのを防ぐ）
		if (attack.m_currentCooldown > 0.0f || attack.m_windupPending)
			return;

		// 受付時間内なら次の段へ、最終段まで来ていれば1段目へ戻す
		combo.m_stage = (combo.m_stage >= MAX_COMBO_STAGE) ? 1 : combo.m_stage + 1;
		combo.m_windowTimer = combo.m_inputWindow;

		attack.m_attackRequested = true;

		// 2段目は回転斬り（Attack2）、それ以外は通常の斬り（Attack1）。
		// 斬撃エフェクトの傾きも段に合わせる（AttackSystemがイベントへ載せる）
		const bool isFinalStage{ combo.m_stage >= MAX_COMBO_STAGE };
		attack.m_effectRotationOffset = isFinalStage ? STAGE2_EFFECT_ROTATION : STAGE1_EFFECT_ROTATION;

		auto& anim{ m_componentManager.get<component::visual::AnimationComponent>(m_playerId) };
		anim.request(isFinalStage
		                 ? constant::AnimationState::Attack2
		                 : constant::AnimationState::Attack1);
	}
} // namespace game::system::combat
