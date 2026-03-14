#include "Result.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "game/ui/Button.h"
#include "game/constant/UI.h"
#include "core/utility/Color.h"
#include "core/ServiceLocator.h"

namespace game::scene
{
    namespace
    {
        // リザルトのUIの配置比率（画面サイズ比）
        constexpr float TITLE_Y_RATIO = 0.20f;          // 画面高さの20%
        constexpr float RETURN_BUTTON_Y_RATIO = 0.60f; // 画面高さの60%
        constexpr float BUTTON_WIDTH_RATIO = 0.15f;    // 画面幅の15%
        constexpr float BUTTON_HEIGHT_RATIO = 0.06f;   // 画面高さの6%
    }

    Result::Result(core::iface::IInputProvider& inputProvider,
        core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen)
        : m_inputProvider{ inputProvider }
        , m_uiRenderer{ uiRenderer }
        , m_screen{ screen }
    {
        setupUI();
    }

    void Result::update(float deltaTime)
    {
        m_uiManager.update();
    }

    void Result::draw()
    {
        const char* TITLE = "リザルト画面";
        int titleWidth = m_uiRenderer.getTextWidth(TITLE);
        int titleX = (m_screen.getWidth() - titleWidth) / 2;
        int titleY = static_cast<int>(m_screen.getHeight() * TITLE_Y_RATIO);
        m_uiRenderer.drawText(titleX, titleY, TITLE, core::utility::Color::WHITE);

        // UI要素を描画
        m_uiManager.draw(m_uiRenderer);
    }

    void Result::setupUI()
    {
        // 画面サイズを取得
        const int SCREEN_WIDTH = m_screen.getWidth();
        const int SCREEN_HEIGHT = m_screen.getHeight();

        // 比率に基づいて実際のピクセル値を計算
        const int BUTTON_WIDTH = static_cast<int>(SCREEN_WIDTH * BUTTON_WIDTH_RATIO);
        const int BUTTON_HEIGHT = static_cast<int>(SCREEN_HEIGHT * BUTTON_HEIGHT_RATIO);
        const int BUTTON_X = (SCREEN_WIDTH - BUTTON_WIDTH) / 2;
        const int RETURN_BUTTON_Y = static_cast<int>(SCREEN_HEIGHT * RETURN_BUTTON_Y_RATIO);

        auto returnButton = std::make_unique<ui::Button>(
            "タイトルへ戻る", BUTTON_X, RETURN_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, m_inputProvider);

        // ボタンが押されたときの処理（Titleシーンへ遷移）
        returnButton->setOnClick([]() {
            auto* sceneManager = core::ServiceLocator::get<game::scene::SceneManager>();
            sceneManager->changeScene(SceneType::Title);
            });

        m_uiManager.addElement(std::move(returnButton));
    }
}