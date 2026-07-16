
#pragma once
#include "platform/window/WindowBase.h"

namespace platform::window
{
	/**
	 * @brief 弾の見た目として飛ばす小型の実OSウィンドウ
	 *
	 * フォーカスを奪わず（WS_EX_NOACTIVATE）、タスクバーにも出さず（WS_EX_TOOLWINDOW）、
	 * 常に最前面（TOPMOST）でゲーム内の弾に追従する。
	 * クライアント領域にはWindowsロゴ風の4枚パネルをGDIで描画する。
	 */
	class ProjectileWindow : public WindowBase
	{
	  public:
		/**
		 * @brief ProjectileWindowのコンストラクタ
		 * @param className ウィンドウクラス名（インスタンスごとに一意にすること）
		 */
		explicit ProjectileWindow(const wchar_t* className) noexcept;

		/**
		 * @brief ウィンドウを生成し、非アクティブ化・最前面などの拡張スタイルを適用する
		 * @param ownerHwnd オーナーウィンドウ（ゲームウィンドウ）
		 * @return 成功時true
		 */
		bool createAsProjectile(HWND ownerHwnd) noexcept;

		/**
		 * @brief フォーカスを奪わずに位置・サイズを変更して表示する
		 * @param x 左上X（デスクトップ座標）
		 * @param y 左上Y（デスクトップ座標）
		 * @param size 一辺のピクセルサイズ
		 */
		void moveTo(int x, int y, int size) noexcept;

		/**
		 * @brief ウィンドウメッセージハンドラ（ロゴ描画・アクティブ化抑制）
		 */
		LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

	  private:
		/**
		 * @brief クライアント領域にWindowsロゴ風の4枚パネルを描画する
		 * @param hwnd ウィンドウハンドル
		 */
		void paintLogo(HWND hwnd) noexcept;
	};
} // namespace platform::window
