#include "Label.h"
#include <utility>

namespace game::ui
{
    Label::Label(std::string text, int x, int y, unsigned int color, int fontSize)
        : m_text{ std::move(text) }
        , m_x{ x }
        , m_y{ y }
        , m_fontSize{ fontSize }
        , m_color{ color }
        , m_visible{ true }
    {
    }

    void Label::update()
    {
        // 状態変化がないため空実装
    }

    void Label::draw(core::iface::IUIRenderer& uiRenderer) const
    {
        if (!m_visible) return;

        uiRenderer.drawText(m_x, m_y, m_text.c_str(), m_color, m_fontSize);
    }

    void Label::setVisible(bool visible)
    {
        m_visible = visible;
    }

    void Label::setText(const std::string& text)
    {
        m_text = text;
    }
}