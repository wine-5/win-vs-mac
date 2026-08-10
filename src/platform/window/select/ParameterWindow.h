#pragma once

#include <windows.h>
#include <string>
#include "platform/window/WindowConstants.h"
#include "platform/window/WebViewWindowBase.h"

namespace platform::window::select
{
	/**
	 * @brief パラメータ1項目分の表示値
	 */
	struct ParameterEntry
	{
		float m_base{ 0.0f };  // 基礎値（playerData.json などの定義値）
		float m_bonus{ 0.0f }; // 装備ファイルによる加算ぶん
	};

	/**
	 * @brief パラメータウィンドウに表示する内容一式
	 *
	 * 項目を増やすたびに refresh() の引数が2つずつ増えるのを避けるためまとめて渡す。
	 * 呼び出し側が値の対応を取り違えないようにする狙いもある
	 */
	struct ParameterStats
	{
		ParameterEntry m_hp{};
		ParameterEntry m_atk{};
		ParameterEntry m_def{};
		ParameterEntry m_spd{};
		ParameterEntry m_attackRange{};     // 攻撃範囲（近接の届く距離）
		ParameterEntry m_crit{};            // クリティカル率（%表記。0.2なら20を入れる）
		ParameterEntry m_projectileSpeed{}; // Window弾の弾速
		ParameterEntry m_projectileRange{}; // Window弾の飛距離
		int m_equippedSlots{ 0 };
	};

	/**
     * @class ParameterWindow
     * @brief ステータス表示ウィンドウ
     */
	class ParameterWindow : public platform::window::WebViewWindowBase
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
		 * @param stats 表示するパラメータ一式
		 */
		void refresh(const ParameterStats& stats) noexcept;

	  protected:
        void onCreateControls(HWND hwnd) override;
        LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

    private:
        // ウィンドウ定数
        static constexpr const wchar_t* ICON_PATH{ L"assets/images/ui/icons/param.ico" };
        static constexpr const wchar_t* WINDOW_CLASS_NAME{ L"ParameterWindowClass" };
        static constexpr const wchar_t* WINDOW_TITLE{ L"パラメータ" };
        static constexpr const wchar_t* PARAMETER_HTML_URL{ L"https://game.web/select/param/param.html" };

    };
} // namespace platform::window::select
