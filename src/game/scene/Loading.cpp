#include "Loading.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IStringConverter.h"
#include <string>
#include <cmath>

namespace game::scene
{
    namespace
    {
        constexpr float TITLE_Y_RATIO = 0.40f;        // 画面高さの40%
        constexpr float TIMER_Y_RATIO = 0.55f;        // 画面高さの55%
    }

    Loading::Loading(core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen)
        : m_uiRenderer{ uiRenderer }
        , m_screen{ screen }
    {
    }

    void Loading::update(float deltaTime)
    {
        m_elapsedTime += deltaTime;

        switch (m_state)
        {
        case State::Playing:
            // 指定時間が経過したらフェードアウト開始
            if (m_elapsedTime >= TEST_LOADING_DURATION)
            {
                m_fade = std::make_unique<ui::FadeTransition>(
                    m_uiRenderer, m_screen, FADE_DURATION, false);
                m_state = State::FadingOut;
            }
            break;

        case State::FadingOut:
            m_fade->update(deltaTime);
            if (m_fade->isFinished())
            {
                auto* sceneManager = core::base::ServiceLocator::get<game::scene::SceneManager>();
                sceneManager->changeScene(SceneType::InGame);
            }
            break;
        }
    }

    void Loading::draw()
    {
        auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };

        const int fontSize{static_cast<int>(m_screen.getHeight() * core::constant::ui::DEFAULT_FONT_SIZE_RATIO)};

        // タイトル表示（文字コード変換して描画）
        std::string title{ "ローディング中..." };
        if (converter)
            title = converter->utf8ToShiftJis(title);
        int titleWidth{m_uiRenderer.getTextWidth(title.c_str(), fontSize)};
        int titleX{(m_screen.getWidth() - titleWidth) / 2};
        int titleY{static_cast<int>(m_screen.getHeight() * TITLE_Y_RATIO)};
        m_uiRenderer.drawText(titleX, titleY, title.c_str(), core::utility::Color::WHITE, fontSize);

        // 経過時間表示（切り上げ）
        float remainingTime{TEST_LOADING_DURATION - m_elapsedTime};
        if (remainingTime < 0.0f) remainingTime = 0.0f;

        std::string timerText = std::to_string(static_cast<int>(std::ceil(remainingTime))) + "秒";
        if (converter)
            timerText = converter->utf8ToShiftJis(timerText);
        int timerWidth{m_uiRenderer.getTextWidth(timerText.c_str(), fontSize)};
        int timerX{(m_screen.getWidth() - timerWidth) / 2};
        int timerY{static_cast<int>(m_screen.getHeight() * TIMER_Y_RATIO)};
        m_uiRenderer.drawText(timerX, timerY, timerText.c_str(), core::utility::Color::YELLOW, fontSize);

        if (m_fade) m_fade->draw(m_uiRenderer, m_screen);
    }
}
