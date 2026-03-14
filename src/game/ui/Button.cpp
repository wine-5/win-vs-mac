#include "Button.h"
#include "core/utility/Color.h"
#include "game/constant/UI.h"

namespace game::ui
{
    Button::Button(const std::string& text, int x, int y, int width, int height,
        core::iface::IInputProvider& inputProvider)
        : m_text(text)
        , m_x{x}
        , m_y{y}
        , m_width{width}
        , m_height{height}
        , m_state{UIState::Normal}
        , m_visible{true}
        , m_wasPressed{false}
        , m_onClick{}
        , m_inputProvider(inputProvider)
    {
    }

    void Button::update()
    {
        if(!m_visible || m_state == UIState::Disabled) return;

        bool isOver = isMouseOver();
        bool isMousePressed = m_inputProvider.isMouseLeftPressed();

        if (isOver)
        {
            if (isMousePressed)
            {
                m_state = UIState::Pressed;
                    m_wasPressed = true;
            }
            else
            {
                // マウスが離されたときにクリック判定
                if (m_wasPressed && m_onClick)
                    m_onClick();

                m_state = UIState::Focused;
                m_wasPressed = false;
            }
        }
        else
        {
            m_state = UIState::Normal;
            m_wasPressed = false;
        }
    }

    void Button::draw(core::iface::IUIRenderer& uiRenderer) const
    {
        if (!m_visible) return;

        // ボタンの背景を描画
        uiRenderer.drawBox(m_x, m_y, m_width, m_height, getColor(), true);

        // ボタンの枠線
        uiRenderer.drawBox(m_x, m_y, m_width, m_height, core::utility::Color::WHITE, false);

        int textWidth = uiRenderer.getTextWidth(m_text.c_str());
        int textX = m_x + (m_width - textWidth) / 2;
        int textY = m_y + (m_height - game::constant::ui::FONT_SIZE_NORMAL) / 2;
        uiRenderer.drawText(textX, textY, m_text.c_str(), core::utility::Color::BLUE);
    }

    void Button::setVisible(bool visible)
    {
        m_visible = visible;
    }

    void Button::setOnClick(std::function<void()> callback)
    {
        m_onClick = callback;
    }

    bool Button::isMouseOver() const
    {
        int mouseX{}, mouseY{};
        m_inputProvider.getMousePosition(mouseX, mouseY);
        return mouseX >= m_x && mouseX <= m_x + m_width &&
            mouseY >= m_y && mouseY <= m_y + m_height;
    }

    unsigned int Button::getColor() const
    {
        switch (m_state)
        {
        case UIState::Normal:   return core::utility::Color::BUTTON_NORMAL;
        case UIState::Focused:  return core::utility::Color::BUTTON_FOCUSED;
        case UIState::Pressed:  return core::utility::Color::BUTTON_PRESSED;
        case UIState::Disabled: return core::utility::Color::BUTTON_DISABLED;
        default:                return core::utility::Color::BUTTON_NORMAL;
        }
    }
}