#pragma once

#include <windows.h>
#include <string>
#include "WindowBase.h"

namespace core::data
{
    struct JobInfo;
}

namespace platform::window
{
    /**
     * @class ParameterWindow
     * @brief ステータス表示ウィンドウ
     */
    class ParameterWindow : public WindowBase
    {
    public:
        /**
         * @brief コンストラクタ
         * @param x ウィンドウの左上角 X 座標
         * @param y ウィンドウの左上角 Y 座標
         * @param width ウィンドウの幅
         * @param height ウィンドウの高さ
         */
        ParameterWindow(int x, int y, int width, int height) noexcept;

        /// @brief デストラクタ
        virtual ~ParameterWindow() noexcept = default;

        /**
         * @brief ステータス情報を更新
         * @param jobInfo 職業情報
         */
        void refresh(const core::data::JobInfo& jobInfo) noexcept;

    protected:
        void onCreateControls(HWND hwnd) override;

    private:
        HWND m_nameLabel{};
        HWND m_skillLabel{};
        HWND m_hpLabel{};
        HWND m_atkLabel{};
        HWND m_defLabel{};
        HWND m_spdLabel{};

        HWND m_nameValue{};
        HWND m_skillValue{};
        HWND m_hpValue{};
        HWND m_atkValue{};
        HWND m_defValue{};
        HWND m_spdValue{};
    };
}
