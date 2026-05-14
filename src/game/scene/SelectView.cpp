#include "SelectView.h"
#include "game/ui/Button.h"
#include "core/constant/UI.h"
#include "core/utility/Color.h"
#include <string>

namespace game::scene
{
    SelectView::SelectView(core::iface::IInputProvider& inputProvider,
        core::iface::IUIRenderer& uiRenderer,
        core::iface::IScreen& screen,
        data::FileEquipmentData& fileEquipmentData,
        core::iface::IJobProvider& jobProvider,
        data::JobSelectionData& jobSelectionData,
        std::function<void()> onGameStart,
        std::function<void(int)> onFileSelect,
        std::function<void(int)> onJobSelect)
        : m_uiRenderer{ uiRenderer }
        , m_screen{ screen }
        , m_fileEquipmentData{ fileEquipmentData }
        , m_jobProvider{ jobProvider }
        , m_jobSelectionData{ jobSelectionData }
    {
        const int screenWidth{ screen.getWidth() };
        const int screenHeight{ screen.getHeight() };
        const int buttonWidth{ static_cast<int>(screenWidth * BUTTON_WIDTH_RATIO) };
        const int buttonHeight{ static_cast<int>(screenHeight * BUTTON_HEIGHT_RATIO) };
        const int buttonX{ (screenWidth - buttonWidth) / 2 };
        const int startButtonY{ static_cast<int>(screenHeight * START_BUTTON_Y_RATIO) };
        const int buttonFontSize{ static_cast<int>(screenHeight * core::constant::ui::DEFAULT_FONT_SIZE_RATIO) };

        auto startButton{ std::make_unique<ui::Button>(
            "ゲームスタート", buttonX, startButtonY, buttonWidth, buttonHeight, inputProvider, buttonFontSize) };
        startButton->setOnClick(std::move(onGameStart));
        m_uiManager.addElement(std::move(startButton));

        constexpr float fileButtonYRatio{ FILE_BUTTON_BASE_Y_RATIO };
        for (int i{ 0 }; i < data::FileEquipmentData::MAX_SLOTS; ++i)
        {
            const int fileButtonY{ static_cast<int>(screenHeight * (fileButtonYRatio - i * FILE_BUTTON_Y_STEP)) };
            const std::string label{ "ファイル" + std::to_string(i + 1) + "を選択" };

            auto fileSelectButton{ std::make_unique<ui::Button>(
                label.c_str(), buttonX, fileButtonY, buttonWidth, buttonHeight, inputProvider, buttonFontSize) };
            fileSelectButton->setOnClick([onFileSelect, i]() { onFileSelect(i); });
            m_uiManager.addElement(std::move(fileSelectButton));
        }

        const int jobButtonX{ static_cast<int>(screenWidth * 0.65f) };
        const int jobButtonBaseY{ static_cast<int>(screenHeight * 0.50f) };
        const int jobCount{ jobProvider.getJobCount() };
        const float jobButtonYStep{ 0.08f };

        for (int i{ 0 }; i < jobCount; ++i)
        {
            const core::constant::JobType jobType{ static_cast<core::constant::JobType>(i) };
            const auto jobInfo{ jobProvider.getJobInfo(jobType) };
            const int jobButtonY{ static_cast<int>(jobButtonBaseY - i * jobButtonYStep * screenHeight) };

            auto jobBtn{ std::make_unique<ui::Button>(
                jobInfo.m_name.c_str(), jobButtonX, jobButtonY, buttonWidth, buttonHeight, inputProvider, buttonFontSize) };
            jobBtn->setOnClick([onJobSelect, i]() { onJobSelect(i); });
            m_uiManager.addElement(std::move(jobBtn));
        }
    }

    void SelectView::update()
    {
        m_uiManager.update();
    }

    void SelectView::draw() const
    {
        const int titleFontSize{ static_cast<int>(m_screen.getHeight() * core::constant::ui::DEFAULT_FONT_SIZE_RATIO) };
        const char* title{ "難易度と武器、ファイルを３つ選択してください" };
        const int titleWidth{ m_uiRenderer.getTextWidth(title, titleFontSize) };
        const int titleX{ (m_screen.getWidth() - titleWidth) / 2 };
        const int titleY{ static_cast<int>(m_screen.getHeight() * TITLE_Y_RATIO) };
        m_uiRenderer.drawText(titleX, titleY, title, core::utility::Color::WHITE, titleFontSize);

        m_uiManager.draw(m_uiRenderer);

        for (int i{ 0 }; i < data::FileEquipmentData::MAX_SLOTS; ++i)
        {
            if (m_fileEquipmentData.hasSelection(i))
            {
                const std::string& fullPath{ m_fileEquipmentData.getFilePath(i) };
                const auto slashPos{ fullPath.rfind('\\') };
                const std::string fileName{ slashPos != std::string::npos
                    ? fullPath.substr(slashPos + 1) : fullPath };
                const std::string text{ "スロット" + std::to_string(i + 1) + ": " + fileName };
                const int textY{ static_cast<int>(m_screen.getHeight() * (FILE_NAME_BASE_Y_RATIO + i * FILE_NAME_Y_STEP)) };
                const int textFontSize{ static_cast<int>(m_screen.getHeight() * core::constant::ui::DEFAULT_FONT_SIZE_RATIO) };
                m_uiRenderer.drawText(FILE_NAME_X, textY, text.c_str(), core::utility::Color::WHITE, textFontSize);
            }
        }

        if (m_jobSelectionData.hasJobSelected())
        {
            const int selectedJobId{ m_jobSelectionData.getSelectedJobId() };
            const core::constant::JobType jobType{ static_cast<core::constant::JobType>(selectedJobId) };
            const auto jobInfo{ m_jobProvider.getJobInfo(jobType) };

            const int paramTextFontSize{ static_cast<int>(m_screen.getHeight() * core::constant::ui::DEFAULT_FONT_SIZE_RATIO) };
            const int paramStartX{ m_screen.getWidth() - 250 };
            const int paramStartY{ static_cast<int>(m_screen.getHeight() * 0.15f) };
            const int paramLineHeight{ static_cast<int>(m_screen.getHeight() * 0.04f) };

            const std::string jobNameText{ "職業: " + jobInfo.m_name };
            m_uiRenderer.drawText(paramStartX, paramStartY, jobNameText.c_str(), core::utility::Color::WHITE, paramTextFontSize);

            const std::string hpText{ "HP: " + std::to_string(static_cast<int>(jobInfo.m_hp)) };
            m_uiRenderer.drawText(paramStartX, paramStartY + paramLineHeight, hpText.c_str(), core::utility::Color::WHITE, paramTextFontSize);

            const std::string atkText{ "ATK: " + std::to_string(static_cast<int>(jobInfo.m_atk)) };
            m_uiRenderer.drawText(paramStartX, paramStartY + paramLineHeight * 2, atkText.c_str(), core::utility::Color::WHITE, paramTextFontSize);

            const std::string defText{ "DEF: " + std::to_string(static_cast<int>(jobInfo.m_def)) };
            m_uiRenderer.drawText(paramStartX, paramStartY + paramLineHeight * 3, defText.c_str(), core::utility::Color::WHITE, paramTextFontSize);

            const std::string spdText{ "SPD: " + std::to_string(static_cast<int>(jobInfo.m_spd)) };
            m_uiRenderer.drawText(paramStartX, paramStartY + paramLineHeight * 4, spdText.c_str(), core::utility::Color::WHITE, paramTextFontSize);
        }
    }

}
