#include "PlayerDeathSystem.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/movement/VelocityComponent.h"
#include "game/component/visual/AnimationComponent.h"
#include "game/event/InGameEvents.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include <algorithm>

namespace
{
	// 死亡アニメが完了イベントを発行しない異常時の保険タイムアウト（秒）。
	// 通常はAnimationFinishedEventで次の段階へ進むので、これは事故防止用の上限
	constexpr float ANIM_FALLBACK_TIMEOUT{ 5.0f };
	// 死亡アニメ完了後、倒れた姿を見せる余韻の長さ（秒）
	constexpr float HOLD_DURATION{ 0.5f };
	// 暗転にかける時間（秒）
	constexpr float FADE_DURATION{ 1.6f };

	// 暗闇の境界のぼかし幅（画面端→中心を0→1とした正規化距離）。
	// 大きいほど「もやっと」広がり、小さいほど輪郭のはっきりした穴が閉じる見え方になる
	constexpr float FADE_SOFT_EDGE{ 0.45f };

	/**
	 * @brief 暗転の進行度と画面端からの距離から、その位置の暗さを求める
	 *
	 * 画面端（distance=0）から先に暗くなり、進行に応じて暗闇の縁が中心へ寄っていく。
	 * progress=1で画面全体が完全な黒になる
	 * @param distance 画面端からの正規化距離（0=画面端、1=中心）
	 * @param progress 暗転の進行度（0〜1）
	 * @return 暗さ（0=透明、1=真っ黒）
	 */
	float darknessAt(float distance, float progress)
	{
		// progress が進むほど、暗闇の縁（暗さ0の位置）が中心（distance=1）を越えて内側へ進む
		const float edge{ progress * (1.0f + FADE_SOFT_EDGE) };
		return std::clamp((edge - distance) / FADE_SOFT_EDGE, 0.0f, 1.0f);
	}
} // namespace

namespace game::system::combat
{
	PlayerDeathSystem::PlayerDeathSystem(core::ecs::ComponentManager& componentManager,
	    core::base::EventBus& eventBus,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_eventBus{ eventBus }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_playerId{ playerId }
	{
		m_subscriptions.push_back(m_eventBus.subscribe<event::PlayerDeadEvent>(
		    [this](const event::PlayerDeadEvent&)
		    { onPlayerDead(); }));

		// 死亡アニメの完了を待って暗転へ進むため、完了イベントを購読する
		m_subscriptions.push_back(m_eventBus.subscribe<event::AnimationFinishedEvent>(
		    [this](const event::AnimationFinishedEvent& e)
		    { onAnimationFinished(e.m_entityId, e.m_state); }));
	}

	void PlayerDeathSystem::onPlayerDead()
	{
		// 多重発行されても最初の一度だけ演出を始める
		if (m_phase != Phase::Idle)
			return;

		m_phase = Phase::DyingAnim;
		m_phaseElapsed = 0.0f;

		// 死んだあとに操作で動かれないよう入力を止める
		if (m_componentManager.has<component::movement::InputComponent>(m_playerId))
			m_componentManager.get<component::movement::InputComponent>(m_playerId).m_locked = true;

		// 死体が横滑りしないよう水平速度を止める。垂直は残し、重力で接地させる
		if (m_componentManager.has<component::movement::VelocityComponent>(m_playerId))
		{
			auto& velocity{ m_componentManager.get<component::movement::VelocityComponent>(m_playerId) };
			velocity.m_velocity.x = 0.0f;
			velocity.m_velocity.z = 0.0f;
		}

		// 死亡アニメを最優先度で要求する。クリップが無ければ待つものが無いので余韻へ直行する
		bool hasDyingClip{ false };
		if (m_componentManager.has<component::visual::AnimationComponent>(m_playerId))
		{
			auto& anim{ m_componentManager.get<component::visual::AnimationComponent>(m_playerId) };
			if (anim.m_clips.contains(constant::AnimationState::Dying))
			{
				anim.m_requested = constant::AnimationState::Dying;
				hasDyingClip = true;
			}
		}
		if (!hasDyingClip)
			m_phase = Phase::Hold;
	}

	void PlayerDeathSystem::onAnimationFinished(core::ecs::EntityId entityId, constant::AnimationState state)
	{
		if (m_phase != Phase::DyingAnim)
			return;
		if (entityId != m_playerId || state != constant::AnimationState::Dying)
			return;

		m_phase = Phase::Hold;
		m_phaseElapsed = 0.0f;
	}

	void PlayerDeathSystem::update(float deltaTime)
	{
		if (m_phase == Phase::Idle || m_phase == Phase::Finished)
			return;

		m_phaseElapsed += deltaTime;

		switch (m_phase)
		{
		case Phase::DyingAnim:
			// 通常はAnimationFinishedEventで抜ける。ここは完了イベントが来ない事故への保険
			if (m_phaseElapsed >= ANIM_FALLBACK_TIMEOUT)
			{
				m_phase = Phase::Hold;
				m_phaseElapsed = 0.0f;
			}
			break;

		case Phase::Hold:
			if (m_phaseElapsed >= HOLD_DURATION)
			{
				m_phase = Phase::FadeOut;
				m_phaseElapsed = 0.0f;
			}
			break;

		case Phase::FadeOut:
			m_fadeProgress = std::clamp(m_phaseElapsed / FADE_DURATION, 0.0f, 1.0f);
			if (m_fadeProgress >= 1.0f)
			{
				// 真っ暗になりきってからシーン遷移を促す
				m_phase = Phase::Finished;
				m_eventBus.publish(event::PlayerDeathSequenceFinishedEvent{});
			}
			break;

		default:
			break;
		}
	}

	void PlayerDeathSystem::draw()
	{
		if (m_fadeProgress <= 0.0f)
			return;

		const int screenW{ m_screen.getWidth() };
		const int screenH{ m_screen.getHeight() };
		const unsigned int black{ core::utility::Color::BLACK };

		// 画面端から中心までを正規化距離0〜1とみなす基準。長辺の半分を取り、
		// 短辺側の枠を描き切ったあとも中心まで距離が伸び続けるようにする
		const float maxInset{ static_cast<float>(std::max(screenW, screenH)) * 0.5f };

		// 枠は短辺の半分までしか描けないため、中心付近の暗さは先に全画面へ敷く
		const float centerDarkness{ darknessAt(1.0f, m_fadeProgress) };
		if (centerDarkness > 0.0f)
		{
			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
			    static_cast<int>(centerDarkness * 255.0f));
			m_uiRenderer.drawBox(0, 0, screenW, screenH, black, true);
		}

		// 全画面が黒で埋まっているなら、この上に重ねる縁のグラデーションは不要
		if (centerDarkness < 1.0f)
		{
			// 画面端から内側へ1pxずつ黒枠を重ね、縁ほど濃いビネットを作る。
			// 刻みを1pxにすることで隙間なく塗り、境界のムラを出さない
			for (int inset{ 0 }; screenW - 2 * inset > 0 && screenH - 2 * inset > 0; ++inset)
			{
				const float distance{ std::min(static_cast<float>(inset) / maxInset, 1.0f) };
				// 全画面の黒に重ねたとき、合成後の暗さが目標値になる分だけを描く
				const float target{ darknessAt(distance, m_fadeProgress) };
				const int alpha{ static_cast<int>((target - centerDarkness) / (1.0f - centerDarkness) * 255.0f) };
				if (alpha <= 0)
					continue;

				m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);
				m_uiRenderer.drawBox(inset, inset, screenW - 2 * inset, screenH - 2 * inset, black, false);
			}
		}

		m_uiRenderer.resetBlendMode();
	}
} // namespace game::system::combat
