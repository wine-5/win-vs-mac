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

	// 段ごとの斬撃エフェクトの傾き（ラジアン）。段ごとに専用のエフェクトを持たせたため、
	// 絵柄側で振りの向きが表現されている。傾きで寝かせる必要は無いので補正は入れない
	const core::Vector3 STAGE1_EFFECT_ROTATION{ 0.0f, 0.0f, 0.0f };
	const core::Vector3 STAGE2_EFFECT_ROTATION{ 0.0f, 0.0f, 0.0f };

	// 斬撃エフェクトの高さの微調整（ワールド単位）。基準は足元で、絵柄が上方向へ
	// 伸びるため既定は補正なし。高すぎる／低すぎる場合はここのYを動かす
	const core::Vector3 EFFECT_POSITION_OFFSET{ 0.0f, 0.0f, 0.0f };
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

		// 前の攻撃が終わるまでは新しい攻撃を出せない（連打で段が飛ぶのを防ぐ）
		const bool isAttacking{ attack.m_currentCooldown > 0.0f || attack.m_windupPending };

		// 振っている最中の入力は捨てずに覚えておく。攻撃アニメが終わってから
		// 入力を受け付けるまでには隙間があり、そこで待機モーションへ戻ってしまうため、
		// 先に押しておけば終わり次第すぐ次の段へ繋がるようにする
		if (isJustPressed && isAttacking)
		{
			combo.m_hasBufferedInput = true;
			return;
		}

		if (isAttacking)
			return;

		// 押した瞬間、または攻撃中に溜めておいた入力があれば次の段を出す
		if (!isJustPressed && !combo.m_hasBufferedInput)
			return;

		combo.m_hasBufferedInput = false;

		// 受付時間内なら次の段へ、最終段まで来ていれば1段目へ戻す
		combo.m_stage = (combo.m_stage >= MAX_COMBO_STAGE) ? 1 : combo.m_stage + 1;
		combo.m_windowTimer = combo.m_inputWindow;

		attack.m_attackRequested = true;

		// 2段目は回転斬り（Attack2）、それ以外は通常の斬り（Attack1）。
		// 斬撃エフェクトの傾きも段に合わせる（AttackSystemがイベントへ載せる）
		const bool isFinalStage{ combo.m_stage >= MAX_COMBO_STAGE };

		// 締めの回転斬りだけ威力を上げる。攻撃力自体は書き換えず倍率で渡すので、
		// 1段目へ戻ったときに元の威力へ自然に戻る
		attack.m_damageMultiplier = isFinalStage ? combo.m_stage2DamageMultiplier : 1.0f;

		attack.m_effectRotationOffset = isFinalStage ? STAGE2_EFFECT_ROTATION : STAGE1_EFFECT_ROTATION;
		attack.m_effectPositionOffset = EFFECT_POSITION_OFFSET;

		// 斬撃エフェクトも段で変える。1段目は振り下ろし、2段目は回転斬りとして作られている
		attack.m_startEffectType = isFinalStage
		                               ? core::constant::EffectType::Player_Slash2
		                               : core::constant::EffectType::Player_Slash1;

		// 振り音も段で変える。1段目と2段目が同じ音だと、コンボが繋がった手応えが出ない
		attack.m_startSeType = isFinalStage
		                           ? core::constant::SeType::PlayerSwing2
		                           : core::constant::SeType::PlayerSwing1;

		auto& anim{ m_componentManager.get<component::visual::AnimationComponent>(m_playerId) };
		anim.request(isFinalStage
		                 ? constant::AnimationState::Attack2
		                 : constant::AnimationState::Attack1);
	}
} // namespace game::system::combat
