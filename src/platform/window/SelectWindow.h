#pragma once

#include <windows.h>
#include <string>
#include "WindowBase.h"
#include <functional>

namespace platform::window
{
    /**
     * @class SelectWindow
     * @brief 難易度選択ウィンドウ
     */
    class SelectWindow : public WindowBase
    {
    public:
        /**
         * @brief コンストラクタ
         * @param x ウィンドウの左上角 X 座標
         * @param y ウィンドウの左上角 Y 座標
         * @param width ウィンドウの幅
         * @param height ウィンドウの高さ
         */
        SelectWindow(int x, int y, int width, int height) noexcept;

        /// @brief デストラクタ
        virtual ~SelectWindow() noexcept = default;

        /**
         * @brief ゲーム開始時のコールバック設定
         * @param callback ゲーム開始を通知するコールバック関数
         */
        void setOnGameStart(std::function<void()> callback) noexcept;

        /**
         * @brief 選択された難易度を取得
         * @return 難易度（0=Easy, 1=Normal, 2=Hard）
         */
        int getSelectedDifficulty() const noexcept;

    protected:
        void onCreateControls(HWND hwnd) override;
        LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

    private:
        enum class Difficulty { Easy = 0, Normal = 1, Hard = 2 };

        HWND m_easyButton{};
        HWND m_normalButton{};
        HWND m_hardButton{};
        HWND m_gameStartButton{};
        Difficulty m_selectedDifficulty{ Difficulty::Normal };
        std::function<void()> m_onGameStart{};

        static constexpr int IDC_EASY_BUTTON = 1001;
        static constexpr int IDC_NORMAL_BUTTON = 1002;
        static constexpr int IDC_HARD_BUTTON = 1003;
        static constexpr int IDC_GAME_START_BUTTON = 1004;
    };
}
