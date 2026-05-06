#include "FadeTransition.h"
#include "core/utility/Color.h"

namespace game::ui
{
	FadeTransition::FadeTransition(core::iface::IUIRenderer& uiRenderer,
		core::iface::IScreen& screen,
		float duration,
		bool isFadeIn)
		: m_uiRenderer{ uiRenderer }
		, m_screen{ screen }
		, m_duration{ duration }
		, m_elapsed{ 0.0f }
		, m_isFadeIn{ isFadeIn }
	{
	}

	void FadeTransition::update(float deltaTime)
	{
		if (m_elapsed < m_duration)
			m_elapsed += deltaTime;
	}

	void FadeTransition::draw() const
	{
		float t{ m_elapsed / m_duration };
		if (t > 1.0f)
			t = 1.0f;

		const float alpha{ m_isFadeIn ? (1.0f - t) : t };
		const int a{ static_cast<int>(alpha * 255) };

		// TODO: IUIRendererにsetAlpha/resetAlphaを追加後、アルファブレンドで描画する
		const unsigned int color{ core::utility::Color::argb(a,0,0,0) };
		m_uiRenderer.drawBox(0, 0, m_screen.getWidth(), m_screen.getHeight(), color, true);

	}

	bool FadeTransition::isFinished() const
	{
		return m_elapsed >= m_duration;
	}
}