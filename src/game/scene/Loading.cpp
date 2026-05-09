#include "Loading.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "core/utility/Color.h"
#include "core/constant/UI.h"
#include "core/ServiceLocator.h"
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
        , m_elapsedTime{ 0.0f }
    {
    }

    void Loading::update(float deltaTime)
    {
        m_elapsedTime += deltaTime;

        // 2秒経過したらInGameシーンへ遷移
        if (m_elapsedTime >= TEST_LOADING_DURATION)
        {
            auto* sceneManager = core::ServiceLocator::get<game::scene::SceneManager>();
            sceneManager->changeScene(SceneType::InGame);
        }
    }

    void Loading::draw()
    {
        const int fontSize{static_cast<int>(m_screen.getHeight() * core::constant::ui::DEFAULT_FONT_SIZE_RATIO)};

        // ローディングタイトル表示
        const char* title{"Loading..."};
        int titleWidth{m_uiRenderer.getTextWidth(title, fontSize)};
        int titleX{(m_screen.getWidth() - titleWidth) / 2};
        int titleY{static_cast<int>(m_screen.getHeight() * TITLE_Y_RATIO)};
        m_uiRenderer.drawText(titleX, titleY, title, core::utility::Color::WHITE, fontSize);

        // 経過時間表示（切り上げ）
        float remainingTime{TEST_LOADING_DURATION - m_elapsedTime};
        if (remainingTime < 0.0f) remainingTime = 0.0f;

        std::string timerText = std::to_string(static_cast<int>(std::ceil(remainingTime))) + "秒";
        int timerWidth{m_uiRenderer.getTextWidth(timerText.c_str(), fontSize)};
        int timerX{(m_screen.getWidth() - timerWidth) / 2};
        int timerY{static_cast<int>(m_screen.getHeight() * TIMER_Y_RATIO)};
        m_uiRenderer.drawText(timerX, timerY, timerText.c_str(), core::utility::Color::YELLOW, fontSize);
    }
}