#include "SelectView.h"
#include "game/ui/Button.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include <string>

namespace game::scene
{
    namespace
    {
        constexpr float FADE_DURATION = 0.5f;
        constexpr float TITLE_Y_RATIO = 0.20f;
        constexpr float START_BUTTON_Y_RATIO = 0.45f;
        constexpr float BUTTON_WIDTH_RATIO = 0.15f;
        constexpr float BUTTON_HEIGHT_RATIO = 0.06f;
    }

    SelectView::SelectView(core::iface::IInputProvider &inputProvider,
        core::iface::IUIRenderer &uiRenderer,
        core::iface::IScreen &screen,
        core::iface::IFileProvider &fileProvider,
        data::FileEquipmentData &fileEquipmentData)
        : m_inputProvider{inputProvider}
        , m_uiRenderer{uiRenderer}
        , m_screen{screen}
        , m_fileProvider{fileProvider}
        , m_fileEquipmentData{fileEquipmentData}
    {
        setupUI();
        m_fade = std::make_unique<ui::FadeTransition>(m_uiRenderer, m_screen, FADE_DURATION, true);
    }

    void SelectView::update(float deltaTime)
    {
        if (m_fade)
            m_fade->update(deltaTime);

        switch (m_state)
        {
        case State::FadeIn:
            if (m_fade && m_fade->isFinished())
            {
                m_fade.reset();
                m_state = State::Idle;
            }
            break;
        case State::Idle:
            m_uiManager.update();
            break;
        case State::FadeOut:
            if (m_fade && m_fade->isFinished())
                m_nextAction = Action::GoToLoading;
            break;
        }
    }

    void SelectView::draw()
    {
        const char *title{"難易度と武器、ファイルを３つ選択してください"};
        int titleWidth{m_uiRenderer.getTextWidth(title)};
        int titleX{(m_screen.getWidth() - titleWidth) / 2};
        int titleY{static_cast<int>(m_screen.getHeight() * TITLE_Y_RATIO)};
        m_uiRenderer.drawText(titleX, titleY, title, core::utility::Color::WHITE);

        m_uiManager.draw(m_uiRenderer);

        for (int i{0}; i < data::FileEquipmentData::MAX_SLOTS; ++i)
        {
            if (m_fileEquipmentData.hasSelection(i))
            {
                const std::string &fullPath{m_fileEquipmentData.getFilePath(i)};
                const auto slashPos{fullPath.rfind('\\')};
                const std::string fileName{slashPos != std::string::npos ? fullPath.substr(slashPos + 1) : fullPath};
                const std::string text{"スロット" + std::to_string(i + 1) + ": " + fileName};
                const int textY{static_cast<int>(m_screen.getHeight() * (0.28f + i * 0.04f))};
                m_uiRenderer.drawText(10, textY, text.c_str(), core::utility::Color::WHITE);
            }
        }

        if (m_fade)
            m_fade->draw();
    }

    bool SelectView::isReadyToChange() const
    {
        return m_nextAction != Action::None;
    }

    SelectView::Action SelectView::getNextAction() const
    {
        return m_nextAction;
    }

    void SelectView::startFadeOut()
    {
        if (m_state != State::Idle)
            return;
        m_fade = std::make_unique<ui::FadeTransition>(m_uiRenderer, m_screen, FADE_DURATION, false);
        m_state = State::FadeOut;
    }

    void SelectView::setupUI()
    {
        const int screenWidth{m_screen.getWidth()};
        const int screenHeight{m_screen.getHeight()};
        const int buttonWidth{static_cast<int>(screenWidth * BUTTON_WIDTH_RATIO)};
        const int buttonHeight{static_cast<int>(screenHeight * BUTTON_HEIGHT_RATIO)};
        const int buttonX{(screenWidth - buttonWidth) / 2};
        const int startButtonY{static_cast<int>(screenHeight * START_BUTTON_Y_RATIO)};

        auto startButton{std::make_unique<ui::Button>(
            "ゲームスタート", buttonX, startButtonY, buttonWidth, buttonHeight, m_inputProvider)};
        startButton->setOnClick([this]()
                                { startFadeOut(); });
        m_uiManager.addElement(std::move(startButton));

        constexpr float fileButtonYRatio{0.35f};
        for (int i{0}; i < data::FileEquipmentData::MAX_SLOTS; ++i)
        {
            const int fileButtonY{static_cast<int>(screenHeight * (fileButtonYRatio - i * 0.08f))};
            const std::string label{"ファイル" + std::to_string(i + 1) + "を選択"};

            auto fileSelectButton{std::make_unique<ui::Button>(
                label.c_str(), buttonX, fileButtonY, buttonWidth, buttonHeight, m_inputProvider)};
            fileSelectButton->setOnClick([this, i]()
                                         {
					const std::string path{ m_fileProvider.selectFile() };
					if (!path.empty())
						m_fileEquipmentData.setFilePath(i, path); });
            m_uiManager.addElement(std::move(fileSelectButton));
        }
    }
}
