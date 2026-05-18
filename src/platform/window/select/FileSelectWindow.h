#pragma once

#include <windows.h>
#include <string>
#include <array>
#include <algorithm>
#include "WindowBase.h"
#include "platform/webview/WebView2Host.h"
#include "game/data/FileExtensionType.h"
#include "game/utility/FileExtensionTypeResolver.h"
#include "game/utility/ExtensionBonusCalculator.h"
#include <functional>

namespace platform::window::select
{
    /**
     * @class FileSelectWindow
     * @brief ファイルスロット選択ウィンドウ
     */
    class FileSelectWindow : public WindowBase
    {
    public:
        /**
         * @brief コンストラクタ
         * @param x ウィンドウの左上角 X 座標
         * @param y ウィンドウの左上角 Y 座標
         * @param width ウィンドウの幅
         * @param height ウィンドウの高さ
         */
        FileSelectWindow(int x, int y, int width, int height) noexcept;

        /// @brief デストラクタ
        virtual ~FileSelectWindow() noexcept = default;

        /**
         * @brief ファイルスロット変更時のコールバック設定
         * @param callback スロットインデックスとファイルパスを受け取るコールバック関数
         */
        void setOnFileSlotChanged(std::function<void(int, const std::string&)> callback) noexcept;

        /**
         * @brief ファイルパスを取得
         * @param slot スロットインデックス（0-2）
         * @return ファイルパス
         */
        std::string getFilePath(int slot) const noexcept;

    protected:
        void onCreateControls(HWND hwnd) override;
        LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

    private:
        static constexpr const wchar_t* ICON_PATH = L"assets/images/ui/icons/file.ico";
        static constexpr int SLOT_COUNT = 3;

        platform::webview::WebView2Host m_webView{};
        std::array<std::string, SLOT_COUNT> m_filePaths{};
        std::array<game::data::FileExtensionType, SLOT_COUNT> m_extensionTypes{
            game::data::FileExtensionType::Unknown,
            game::data::FileExtensionType::Unknown,
            game::data::FileExtensionType::Unknown
        };
        std::function<void(int, const std::string&)> m_onFileSlotChanged{};

        void handleMessage(const std::string& json) noexcept;
        void openFileDialog(int slotIndex);
        void sendSlotsRefresh() noexcept;
        void sendBonusInfo() noexcept;
    };
}
