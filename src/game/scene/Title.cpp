#include "Title.h"
#include "SceneManager.h"
#include "SceneType.h"
#include "game/ui/Button.h"
#include "game/constant/UI.h"
#include "core/utility/Color.h"
#include "core/ServiceLocator.h"
#include <cstdlib>

namespace game::scene
{
    namespace
    {
        // タイトルのUIの配置比率（画面サイズ比）
        constexpr float TITLE_Y_RATIO = 0.20f;          // 画面高さの20%
        constexpr float START_BUTTON_Y_RATIO = 0.45f;  // 画面高さの45%
        constexpr float EXIT_BUTTON_Y_RATIO = 0.60f;   // 画面高さの60%
        constexpr float BUTTON_WIDTH_RATIO = 0.15f;    // 画面幅の15%
        constexpr float BUTTON_HEIGHT_RATIO = 0.06f;   // 画面高さの6%
    }

    Title::Title(core::iface::IInputProvider& inputProvider,
        core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen)
        : m_inputProvider{inputProvider}
        , m_uiRenderer{uiRenderer}
        , m_screen{screen}
    {
        setupUI();
    }

    void Title::update(float deltaTime)
    {
        m_uiManager.update();
    }

    void Title::draw()
    {
        const char* title{"タイトルシーン（ここにタイトルを書く予定)"};
        int titleWidth{m_uiRenderer.getTextWidth(title)};
        int titleX{(m_screen.getWidth() - titleWidth) / 2};
        int titleY{static_cast<int>(m_screen.getHeight() * TITLE_Y_RATIO)};
        m_uiRenderer.drawText(titleX, titleY, title, core::utility::Color::WHITE);

        // UI要素を描画
        m_uiManager.draw(m_uiRenderer);
    }

    void Title::setupUI()
    {
        // 画面サイズを取得
        const int screenWidth = m_screen.getWidth();
        const int screenHeight = m_screen.getHeight();
        
        // 比率に基づいて実際のピクセル値を計算
        const int buttonWidth = static_cast<int>(screenWidth * BUTTON_WIDTH_RATIO);
        const int buttonHeight = static_cast<int>(screenHeight * BUTTON_HEIGHT_RATIO);
        const int buttonX = (screenWidth - buttonWidth) / 2;
        const int startButtonY = static_cast<int>(screenHeight * START_BUTTON_Y_RATIO);
        const int exitButtonY = static_cast<int>(screenHeight * EXIT_BUTTON_Y_RATIO);

        auto startButton{std::make_unique<ui::Button>(
            "ステージ選択へ", buttonX, startButtonY, buttonWidth, buttonHeight, m_inputProvider)};

        // ボタンが押されたときの処理
        startButton->setOnClick([]() {
            // ServiceLocatorからSceneManagerを取得してシーン遷移
            auto* sceneManager = core::ServiceLocator::get<game::scene::SceneManager>();
            sceneManager->changeScene(SceneType::StageSelect);
            });

        m_uiManager.addElement(std::move(startButton));

        // 終了ボタンの作成
        auto exitButton{std::make_unique<ui::Button>(
            "ゲームをやめる", buttonX, exitButtonY, buttonWidth, buttonHeight, m_inputProvider)};

        exitButton->setOnClick([]() {
            std::exit(0);  // ゲーム終了
            });

        m_uiManager.addElement(std::move(exitButton));
    }
}