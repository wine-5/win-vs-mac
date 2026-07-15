#pragma once

#include <windows.h>
#include <string>
#include "platform/window/WindowConstants.h"
#include "platform/window/WindowBase.h"
#include "platform/webview/WebView2Host.h"

namespace platform::window::select
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
         * @param baseHp 基礎 HP
         * @param baseAtk 基礎 ATK
         * @param baseDef 基礎 DEF
         * @param baseSpd 基礎 SPD
         * @param bonusHp ファイル装備ボーナス HP
         * @param bonusAtk ファイル装備ボーナス ATK
         * @param bonusDef ファイル装備ボーナス DEF
         * @param bonusSpd ファイル装備ボーナス SPD
         * @param jobNameSjis 職業名（Shift-JIS）
         * @param skillNameSjis スキル名（Shift-JIS）
         * @param equippedSlots 装備中スロット数
         */
        void refresh(
            float baseHp, float baseAtk, float baseDef, float baseSpd,
            float bonusHp, float bonusAtk, float bonusDef, float bonusSpd,
            const std::string& jobNameSjis,
            const std::string& skillNameSjis,
            int equippedSlots) noexcept;

    protected:
        void onCreateControls(HWND hwnd) override;
        LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

    private:
        // ウィンドウ定数
        static constexpr const wchar_t* ICON_PATH{ L"assets/images/ui/icons/param.ico" };
        static constexpr const wchar_t* WINDOW_CLASS_NAME{ L"ParameterWindowClass" };
        static constexpr const wchar_t* WINDOW_TITLE{ L"パラメータ" };
        static constexpr const wchar_t* PARAMETER_HTML_URL{ L"https://game.web/select/param/param.html" };

        // 文字コード定数
        static constexpr int SJIS_CODE_PAGE{ 932 };
        static constexpr int UTF8_CODE_PAGE{ CP_UTF8 };

        platform::webview::WebView2Host m_webView{};
    };
} // namespace platform::window::select
