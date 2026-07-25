#include "LowHealthVignetteView.h"
#include "LowHealthPulse.h"
#include "core/constant/UI.h"
#include "core/interface/IResourceManager.h"
#include "core/utility/Log.h"
#include "game/component/combat/HealthComponent.h"
#include <algorithm>

namespace
{
	constexpr const char* VIGNETTE_IMAGE_ID{ "vignette-lowhp" };

	// 画像自体が最大30%の赤を持つため、ここでさらに掛ける不透明度は控えめでよい。
	// ボス覚醒の赤ビネットと重なる場面があり、強くしすぎると画面が真っ赤になる
	constexpr int VIGNETTE_MAX_ALPHA{ 200 };

	// 警告域に入った瞬間の下限。0から始めると点滅の谷で完全に消えて明滅が唐突に見える
	constexpr float VIGNETTE_MIN_INTENSITY{ 0.35f };
} // namespace

namespace game::ui::ingame
{
	LowHealthVignetteView::LowHealthVignetteView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::ComponentManager& componentManager,
	    core::iface::IResourceManager& resourceManager)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_componentManager{ componentManager }
	{
		m_imageHandle = resourceManager.loadImageById(VIGNETTE_IMAGE_ID);
		if (m_imageHandle == -1)
			core::log::error("低HP警告のビネット画像 '{}' の読み込みに失敗しました", VIGNETTE_IMAGE_ID);
	}

	void LowHealthVignetteView::draw(core::ecs::EntityId playerId)
	{
		if (m_imageHandle == -1)
			return;
		if (!m_componentManager.has<component::combat::HealthComponent>(playerId))
			return;

		const auto& health{ m_componentManager.get<component::combat::HealthComponent>(playerId) };
		if (health.m_maxHp <= 0.0f)
			return;

		const float ratio{ std::clamp(health.m_currentHp / health.m_maxHp, 0.0f, 1.0f) };
		if (!low_health::isLow(ratio))
			return;

		const float elapsed{ std::chrono::duration<float>(
			std::chrono::steady_clock::now() - m_startTime)
			    .count() };

		// 「HPが減るほど濃く」と「HPバーと同じ点滅」を掛け合わせる。
		// 警告域に入った直後は薄く、瀕死に近づくほど濃く速く脈打つ
		const float danger{ low_health::computeDanger(ratio) };
		const float wave{ low_health::computeWave(ratio, elapsed) };
		const float intensity{ (VIGNETTE_MIN_INTENSITY + (1.0f - VIGNETTE_MIN_INTENSITY) * wave) * danger };

		const int alpha{ static_cast<int>(VIGNETTE_MAX_ALPHA * intensity) };
		if (alpha <= 0)
			return;

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);
		m_uiRenderer.drawImage(m_imageHandle, 0, 0, m_screen.getWidth(), m_screen.getHeight());
		m_uiRenderer.resetBlendMode();
	}
} // namespace game::ui::ingame
