#include "Button.h"
#include "core/utility/Color.h"

namespace game::ui
{
    Button::Button(const std::string& text, int x, int y, int width, int height,
        core::iface::IInputProvider& inputProvider)
        : m_text(text)
        , m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height)
        , m_state(UIState::Normal)
        , m_visible(true)
        , m_wasPressed(false)
        , m_onClick(nullptr)
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
            m_state == UIState::Normal;
            m_wasPressed = false;
        }
    }

    void Button::draw(core::iface::IUIRenderer& uiRenderer) const
    {
        if (!m_visible) return;

        // ボタンの背景を描画
        uiRenderer.drawBox(m_x, m_y, m_width, m_height, getColor(), true);

        // ボタンの枠線
        uiRenderer.drawBox(m_x, m_y, m_width, m_height, core::utility::Color::White, false);
    }
}