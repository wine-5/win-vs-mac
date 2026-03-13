#include "TitleScene.h"
#include "InGameScene.h"
#include "game/ui/Button.h"
#include "game/constant/UI.h"
#include "core/utility/Color.h"
#include "core/ServiceLocator.h"
#include <cstdlib>
#include "game/scene/SceneManager.h"

namespace game::scene
{
	namespace
	{
        // タイトルのUIの定数
        constexpr int TITLE_Y = 150;
        constexpr int START_BUTTON_Y = 300;
        constexpr int EXIT_BUTTON_Y = 400;
        constexpr int BUTTON_WIDTH = 200;
        constexpr int BUTTON_HEIGHT = 50;
	}

    TitleScene::TitleScene(core::iface::IInputProvider& inputProvider,
        core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen,
        SceneManager& sceneManager)
        : m_inputProvider(inputProvider)
        , m_uiRenderer(uiRenderer)
        , m_screen(screen)
        , m_sceneManager(sceneManager)
    {
        setupUI();
    }

    void TitleScene::update(float deltaTime)
    {
        m_uiManager.update();
    }

    void TitleScene::draw()
    {
        const char* title = "タイトルシーン（ここにタイトルを書く予定)";
        int titleWidth = m_uiRenderer.getTextWidth(title);
        int titleX = (m_screen.getWidth() - titleWidth) / 2;
        m_uiRenderer.drawText(titleX, TITLE_Y, title, core::utility::Color::WHITE);

        //  UI要素を描画
        m_uiManager.draw(m_uiRenderer);
    }

    void TitleScene::setupUI()
    {
        int buttonX = (m_screen.getWidth() - BUTTON_WIDTH) / 2;

        // スタートボタンの作成
        auto startButton = std::make_unique<ui::Button>(
            "開始", buttonX, START_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, m_inputProvider);
        
        // ボタンが押されたときに参照する関数を登録
        startButton->setOnClick([this]() {
            // InGameSceneに遷移
            // ここは後で書く
            });

        m_uiManager.addElement(std::move(startButton));

        // 終了ボタンの作成
        auto exitButton = std::make_unique<ui::Button>(
            "ゲームをやめる", buttonX, EXIT_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, m_inputProvider);

        exitButton->setOnClick([]() {
            std::exit(0);  // ゲーム終了
            });

        m_uiManager.addElement(std::move(exitButton));
    }
}