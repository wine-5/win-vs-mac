#include "MacAwakenEffectSystem.h"
#include "game/component/camera/CameraEffectComponent.h"
#include "game/component/movement/TransformComponent.h"
#include "game/component/movement/InputComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/event/InGameEvents.h"
#include "game/constant/MacAwakenTiming.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include <cmath>
#include <algorithm>

namespace
{
	// 演出タイムライン（秒）は game/constant/MacAwakenTiming.h で MacAISystem と共有する
	namespace timing = game::constant::mac_awaken;

	constexpr float MAC_LOOK_HEIGHT{ 180.0f }; // ボスの足元でなく胴体〜頭あたりを注視する高さ

	constexpr float SHAKE_FREQUENCY{ 38.0f };        // 揺れの振動周波数
	constexpr float SHAKE_Y_FREQUENCY_RATIO{ 1.3f }; // Y軸をX軸と別周波数にする比（円状でなく不規則な揺れにする）

	constexpr float VIGNETTE_PULSE_FREQ{ 9.0f };     // 濃さの脈動（心拍のようなドクドク感）の周波数
	constexpr float VIGNETTE_PULSE_AMOUNT{ 0.25f };  // 脈動で濃さが揺れる割合
	constexpr float VIGNETTE_JITTER_AMOUNT{ 0.15f }; // フレームごとのランダムなちらつき割合
	constexpr int VIGNETTE_BAND{ 200 };              // 画面端から内側へ何pxまで赤くするか（基準値）
	constexpr int VIGNETTE_BAND_JITTER{ 60 };        // 帯の幅をランダムに伸縮させる量（もやもや感）
	constexpr int VIGNETTE_STEP{ 4 };                // ビネットの帯を描く刻み幅（px）

	/**
	 * @brief 滑らかな0→1補間（smoothstep）。等速より緩急がついてカメラの寄りが上品になる
	 * @param t 進行度（0〜1）
	 * @return 補間値（0〜1）
	 */
	float smoothstep(float t)
	{
		t = std::clamp(t, 0.0f, 1.0f);
		return t * t * (3.0f - 2.0f * t);
	}

	// 演出の強度プリセット。トリガーごとにシェイク振幅・ビネットの濃さと色を個別に決める。
	struct CinematicIntensity
	{
		float m_shakeStrength;        // ホールド中のシェイクの最大振幅（ワールド単位）
		float m_vignetteAlpha;        // ビネットの最大濃さ（0〜1）
		unsigned int m_vignetteColor; // ビネットの色（0xRRGGBB）
	};

	// 出現：初登場なので控えめ（軽い揺れ・オレンジの縁）。
	// 覚醒との差を色で付ける。オレンジ→赤と段階を踏ませることで、
	// 同じ演出でも「まだ本気ではない」「本気になった」が一目で伝わる
	constexpr CinematicIntensity APPEARANCE_INTENSITY{
		12.0f, 0.45f, core::utility::Color::rgb(230, 110, 20)
	};

	// 覚醒：本気モードなので強め。色は最も危険を示す赤で振り切る
	constexpr CinematicIntensity AWAKEN_INTENSITY{
		22.0f, 0.85f, core::utility::Color::rgb(200, 0, 0)
	};
} // namespace

namespace game::system::visual
{
	MacAwakenEffectSystem::MacAwakenEffectSystem(core::ecs::ComponentManager& componentManager,
	    core::base::EventBus& eventBus,
	    core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::EntityId playerId)
	    : m_componentManager{ componentManager }
	    , m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_playerId{ playerId }
	{
		// ボスへ寄る同一のカメラ演出を、覚醒（フェーズ移行）と出現の2つのトリガーから起動する。
		// カメラの動きは共通だが、シェイク・赤ビネットの強度はトリガーごとのプリセットで切り替える
		const auto start{ [this](core::ecs::EntityId bossId, const CinematicIntensity& intensity)
			{
			    m_macId = bossId;
			    m_elapsedTime = 0.0f;
			    m_isPlaying = true;
			    m_shakeStrength = intensity.m_shakeStrength;
			    m_vignetteStrength = intensity.m_vignetteAlpha;
			    m_vignetteColor = intensity.m_vignetteColor;
			    setMacInvincible(true);
			} };

		// 覚醒（HP閾値でのフェーズ移行）：強め
		m_subscriptions.push_back(eventBus.subscribe<event::MacPhaseTransitionEvent>(
		    [start](const event::MacPhaseTransitionEvent& e)
		    { start(e.m_entityId, AWAKEN_INTENSITY); }));

		// 出現（雑魚を全滅させてボスが登場）：控えめ
		m_subscriptions.push_back(eventBus.subscribe<event::BossAppearedEvent>(
		    [start](const event::BossAppearedEvent& e)
		    { start(e.m_entityId, APPEARANCE_INTENSITY); }));
	}

	void MacAwakenEffectSystem::update(float deltaTime)
	{
		if (!m_componentManager.has<component::camera::CameraEffectComponent>(m_playerId))
			return;

		auto& effect{ m_componentManager.get<component::camera::CameraEffectComponent>(m_playerId) };

		// 演出中でなければ全チャンネルを無効値へ戻して終了
		if (!m_isPlaying)
		{
			effect.m_cinematicBlend = 0.0f;
			effect.m_awakenShakeOffset = core::Vector3{ 0.0f, 0.0f, 0.0f };
			m_vignetteAlpha = 0.0f;
			return;
		}

		m_elapsedTime += deltaTime;

		// 演出終了：チャンネルを戻し、入力ロックを解除する
		if (m_elapsedTime >= timing::TOTAL_TIME || !m_componentManager.has<component::movement::TransformComponent>(m_macId))
		{
			m_isPlaying = false;
			effect.m_cinematicBlend = 0.0f;
			effect.m_awakenShakeOffset = core::Vector3{ 0.0f, 0.0f, 0.0f };
			m_vignetteAlpha = 0.0f;
			setMacInvincible(false);
			if (m_componentManager.has<component::movement::InputComponent>(m_playerId))
				m_componentManager.get<component::movement::InputComponent>(m_playerId).m_locked = false;
			return;
		}

		// 演出中はプレイヤーの操作を受け付けない
		if (m_componentManager.has<component::movement::InputComponent>(m_playerId))
			m_componentManager.get<component::movement::InputComponent>(m_playerId).m_locked = true;

		// 注視先＝ボスの胴体あたり（毎フレーム追従。演出中にボスは動かないが位置ズレに備える）
		const auto& macTransform{ m_componentManager.get<component::movement::TransformComponent>(m_macId) };
		effect.m_cinematicTarget = core::Vector3{
			macTransform.m_position.x,
			macTransform.m_position.y + MAC_LOOK_HEIGHT,
			macTransform.m_position.z
		};

		// --- タイムラインからブレンド量（カメラの寄り具合）を決める ---
		float blend{ 0.0f };
		float holdIntensity{ 0.0f }; // ホールド演出（シェイク・ビネット）の強さ（0〜1）
		if (m_elapsedTime < timing::ZOOM_IN_TIME)
		{
			// ①ズームイン：0→1へ滑らかに寄る
			blend = smoothstep(m_elapsedTime / timing::ZOOM_IN_TIME);
		}
		else if (m_elapsedTime < timing::ZOOM_IN_TIME + timing::HOLD_TIME)
		{
			// ②ホールド：寄ったままシェイク＋赤ビネット
			blend = 1.0f;
			holdIntensity = 1.0f;
		}
		else
		{
			// ③ズームアウト：1→0へ滑らかに引く
			const float t{ (m_elapsedTime - timing::ZOOM_IN_TIME - timing::HOLD_TIME) / timing::ZOOM_OUT_TIME };
			blend = 1.0f - smoothstep(t);
		}
		effect.m_cinematicBlend = blend;

		// --- シェイク（ホールド中のみ） ---
		if (holdIntensity > 0.0f)
		{
			const float amplitude{ m_shakeStrength * holdIntensity };
			const float phase{ m_elapsedTime * SHAKE_FREQUENCY };
			effect.m_awakenShakeOffset = core::Vector3{
				std::sin(phase) * amplitude,
				std::cos(phase * SHAKE_Y_FREQUENCY_RATIO) * amplitude,
				0.0f
			};
		}
		else
		{
			effect.m_awakenShakeOffset = core::Vector3{ 0.0f, 0.0f, 0.0f };
		}

		// --- 赤ビネット（ホールド中のみ）：脈動＋ランダムちらつきで絵を単調にしない ---
		if (holdIntensity > 0.0f)
		{
			// 心拍のような脈動（sin）＋フレームごとの微細な乱れ
			const float pulse{ 1.0f - VIGNETTE_PULSE_AMOUNT * (0.5f + 0.5f * std::sin(m_elapsedTime * VIGNETTE_PULSE_FREQ)) };
			std::uniform_real_distribution<float> jitterDist{ 1.0f - VIGNETTE_JITTER_AMOUNT, 1.0f };
			m_vignetteAlpha = m_vignetteStrength * holdIntensity * pulse * jitterDist(m_rng);
		}
		else
		{
			m_vignetteAlpha = 0.0f;
		}
	}

	void MacAwakenEffectSystem::setMacInvincible(bool isInvincible) noexcept
	{
		if (!m_componentManager.has<component::combat::HealthComponent>(m_macId))
			return;
		m_componentManager.get<component::combat::HealthComponent>(m_macId).m_isInvincible = isInvincible;
	}

	void MacAwakenEffectSystem::draw()
	{
		if (m_vignetteAlpha <= 0.0f)
			return;

		const int screenW{ m_screen.getWidth() };
		const int screenH{ m_screen.getHeight() };

		// 帯の幅をフレームごとにランダムに伸縮させ、縁の「もやもや」感を出す
		std::uniform_int_distribution<int> bandDist{ VIGNETTE_BAND - VIGNETTE_BAND_JITTER, VIGNETTE_BAND + VIGNETTE_BAND_JITTER };
		const int band{ bandDist(m_rng) };

		// 画面端から内側へ、赤い枠を重ねてビネット（縁が濃く中心へ薄れる）を作る。
		// 端から離れるほどアルファを下げ、もやもやとした境界にする。
		for (int inset{ 0 }; inset < band; inset += VIGNETTE_STEP)
		{
			// inset=0（画面端）で最大、bandで0へ線形に薄れる
			const float edgeFactor{ 1.0f - static_cast<float>(inset) / static_cast<float>(band) };
			const int alpha{ static_cast<int>(m_vignetteAlpha * edgeFactor * 255.0f) };
			if (alpha <= 0)
				continue;

			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);
			// 塗りつぶさない矩形（枠）を1段ずつ内側へ描く（幅・高さは両端ぶん詰める）
			m_uiRenderer.drawBox(inset, inset, screenW - 2 * inset, screenH - 2 * inset,
			    m_vignetteColor, false);
		}

		m_uiRenderer.resetBlendMode();
	}
} // namespace game::system::visual
