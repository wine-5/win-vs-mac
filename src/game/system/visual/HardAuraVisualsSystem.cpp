#include "HardAuraVisualsSystem.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IResourceManager.h"
#include "core/utility/Log.h"
#include "core/utility/Vector3.h"
#include "game/component/EnemyTypeComponent.h"
#include "game/component/combat/ColliderComponent.h"
#include "game/component/movement/TransformComponent.h"
#include <cmath>

namespace
{
	// 背景の星と同じ白い光の画像を、赤く着色して使い回す
	constexpr const char* GLOW_IMAGE_ID{ "sky-glow" };

	// オーラの色（0xRRGGBB）。純赤だと黒背景で沈むため、わずかに橙へ寄せて熱を持たせる
	constexpr unsigned int AURA_COLOR{ 0xFF2810u };

	// オーラの大きさ。敵のコライダーの最も長い辺に対する倍率。
	// 光の画像は中心が明るく周辺が薄いうえ、明るい中心は敵モデルに隠れてしまう。
	// 大きく描くほど「輪郭からはみ出す部分」に濃いところが来るので、体格より十分大きく取る
	constexpr float AURA_SIZE_SCALE{ 3.0f };

	// 内側にもう一枚重ねる際の倍率。輪郭のすぐ外を濃くして縁取りに見せる
	constexpr float AURA_CORE_SIZE_SCALE{ 1.8f };

	// コライダーの高さに対する、オーラ中心の位置（0.0＝足元、1.0＝頭頂）
	constexpr float AURA_HEIGHT_RATIO{ 0.5f };

	// 脈動の速さ（ラジアン/秒）と、明るさの下限・振れ幅
	constexpr float PULSE_SPEED{ 3.0f };
	constexpr int PULSE_BRIGHTNESS_BASE{ 170 };
	constexpr int PULSE_BRIGHTNESS_RANGE{ 85 };

	// 大きさの脈動幅（1.0を中心に±この割合で伸縮する）
	constexpr float PULSE_SIZE_RANGE{ 0.08f };
} // namespace

namespace game::system::visual
{
	HardAuraVisualsSystem::HardAuraVisualsSystem(core::ecs::ComponentManager& componentManager,
	    core::iface::IRenderer& renderer,
	    core::iface::IResourceManager& resourceManager,
	    bool isHard)
	    : m_componentManager{ componentManager }
	    , m_renderer{ renderer }
	    , m_isHard{ isHard }
	{
		// Normalでは一度も描かないので画像を抱えない
		if (!m_isHard)
			return;

		m_glowHandle = resourceManager.loadImageById(GLOW_IMAGE_ID);
		if (m_glowHandle == -1)
			core::log::error("Hardオーラの光の画像 '{}' の読み込みに失敗しました", GLOW_IMAGE_ID);
	}

	void HardAuraVisualsSystem::update(float deltaTime)
	{
		if (!m_isHard)
			return;

		m_elapsedTime += deltaTime;
	}

	void HardAuraVisualsSystem::draw()
	{
		if (!m_isHard || m_glowHandle == -1)
			return;

		// 明るさと大きさを同じ位相で揺らし、脈打つ心臓のように見せる
		const float pulse{ 0.5f + 0.5f * std::sin(m_elapsedTime * PULSE_SPEED) };
		const int brightness{ PULSE_BRIGHTNESS_BASE + static_cast<int>(PULSE_BRIGHTNESS_RANGE * pulse) };
		const float sizeScale{ 1.0f - PULSE_SIZE_RANGE + 2.0f * PULSE_SIZE_RANGE * pulse };

		// 敵にだけ付くEnemyTypeComponentを起点に走査する（プレイヤーや弾を巻き込まない）
		for (const auto entityId : m_componentManager.getAllEntities<component::EnemyTypeComponent>())
		{
			auto* transform{ m_componentManager.tryGet<component::movement::TransformComponent>(entityId) };
			auto* collider{ m_componentManager.tryGet<component::combat::ColliderComponent>(entityId) };
			if (!transform || !collider)
				continue;

			// 体格に合わせて包む。人型の敵は幅より背が高いので、
			// 幅だけを基準にすると体の上下がオーラからはみ出してしまう。最も長い辺を使う
			const float width{ (collider->m_size.x > collider->m_size.z)
				                   ? collider->m_size.x
				                   : collider->m_size.z };
			const float extent{ (width > collider->m_size.y) ? width : collider->m_size.y };
			if (extent <= 0.0f)
				continue;

			const core::Vector3 center{
				transform->m_position.x + collider->m_offset.x,
				transform->m_position.y + collider->m_size.y * AURA_HEIGHT_RATIO,
				transform->m_position.z + collider->m_offset.z
			};

			// 大きく薄い外側と、輪郭のすぐ外を濃くする内側の2枚を重ねる。
			// 光は敵モデルに隠れるため、実際に見えるのは輪郭からはみ出した部分だけになる
			m_renderer.drawGlowBillboard(m_glowHandle, center,
			    extent * AURA_SIZE_SCALE * sizeScale, 0.0f, brightness, AURA_COLOR);
			m_renderer.drawGlowBillboard(m_glowHandle, center,
			    extent * AURA_CORE_SIZE_SCALE * sizeScale, 0.0f, brightness, AURA_COLOR);
		}
	}
} // namespace game::system::visual
