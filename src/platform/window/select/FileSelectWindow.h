#pragma once

#include <windows.h>
#include <string>
#include <array>
#include <algorithm>
#include "platform/window/WindowConstants.h"
#include "platform/window/WebViewWindowBase.h"
#include "core/data/FileExtensionType.h"
#include "game/utility/FileExtensionTypeResolver.h"
#include <functional>

namespace core::iface
{
	class IResourceManager;
} // namespace core::iface

namespace platform::window::select
{
    /**
     * @class FileSelectWindow
     * @brief ファイルスロット選択ウィンドウ
     */
	class FileSelectWindow : public platform::window::WebViewWindowBase
	{
    public:
	  /**
	   * @brief コンストラクタ
	   * @param x ウィンドウの左上角 X 座標
	   * @param y ウィンドウの左上角 Y 座標
	   * @param width ウィンドウの幅
	   * @param height ウィンドウの高さ
	   * @param resourceManager 拡張子ボーナスの説明文生成に使うリソース管理インターフェース
	   */
	  FileSelectWindow(int x, int y, int width, int height,
		  core::iface::IResourceManager& resourceManager) noexcept;

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
        // ウィンドウ定数
        static constexpr const wchar_t* ICON_PATH{ L"assets/images/ui/icons/file.ico" };
        static constexpr const wchar_t* WINDOW_CLASS_NAME{ L"FileSelectWindowClass" };
        static constexpr const wchar_t* WINDOW_TITLE{ L"ファイル選択" };
        static constexpr const wchar_t* FILE_SELECT_HTML_URL{ L"https://game.web/select/file/file.html" };
        static constexpr int SLOT_COUNT{ 3 };

        // ファイルダイアログフィルター
        static constexpr const char* FILE_DIALOG_FILTER{ "All Files\0*.*\0" };

        // ファイル拡張子タイプ名
        static constexpr const char* EXT_TYPE_NAME_EXECUTABLE{ "Executable" };
        static constexpr const char* EXT_TYPE_NAME_DOCUMENT{ "Document" };
        static constexpr const char* EXT_TYPE_NAME_IMAGE{ "Image" };
        static constexpr const char* EXT_TYPE_NAME_AUDIO{ "Audio" };
        static constexpr const char* EXT_TYPE_NAME_ARCHIVE{ "Archive" };
        static constexpr const char* EXT_TYPE_NAME_UNKNOWN{ "Unknown" };

        std::array<std::string, SLOT_COUNT> m_filePaths{};
		core::iface::IResourceManager& m_resourceManager;

		std::array<core::data::FileExtensionType, SLOT_COUNT> m_extensionTypes{
			core::data::FileExtensionType::Unknown,
			core::data::FileExtensionType::Unknown,
			core::data::FileExtensionType::Unknown
		};
		std::function<void(int, const std::string&)> m_onFileSlotChanged{};

        void handleMessage(const std::string& json) noexcept;
        void openFileDialog(int slotIndex);
        void sendSlotsRefresh() noexcept;
        void sendBonusInfo() noexcept;
    };
} // namespace platform::window::select
