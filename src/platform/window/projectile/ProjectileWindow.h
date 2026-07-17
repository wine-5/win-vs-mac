#pragma once
#include "platform/window/WindowBase.h"

// GDI+ の前方宣言（ヘッダ汚染を避けるため実体は cpp でインクルードする）
namespace Gdiplus
{
	class Image;
}

namespace platform::window
{
	/**
	 * @brief 弾の見た目として飛ばす小型の実OSウィンドウ
	 *
	 * 本物のタイトルバー・閉じるボタンを持つが、操作には一切反応しない
	 * （閉じる・ドラッグ・フォーカス奪取をすべて無効化）。
	 * タスクバーには出さず（WS_EX_TOOLWINDOW）、常に最前面でゲーム内の弾に追従する。
	 * クライアント領域にはロゴ画像を全面描画する（未設定時はGDIでロゴ風パネルを描く）。
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
		 *
		 * ロゴ描画領域（クライアント）が size×size になるよう枠込みサイズを逆算する。
		 * サイズが小さいときはタイトルバーを外し、ロゴが最後まで見えるようにする。
		 * @param centerX ロゴ中心X（デスクトップ座標）
		 * @param centerY ロゴ中心Y（デスクトップ座標）
		 * @param size ロゴ領域の一辺のピクセルサイズ
		 */
		void moveTo(int centerX, int centerY, int size) noexcept;

		/**
		 * @brief クライアント領域に全面描画するロゴ画像を設定する
		 * @param image GDI+画像（所有権は呼び出し側＝マネージャが持つ）
		 */
		void setLogoImage(Gdiplus::Image* image) noexcept;

		/**
		 * @brief ウィンドウメッセージハンドラ（ロゴ描画・操作無効化）
		 */
		LRESULT onMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;

	  private:
		/**
		 * @brief クライアント領域にロゴを描画する
		 * @param hwnd ウィンドウハンドル
		 */
		void paintLogo(HWND hwnd) noexcept;

		Gdiplus::Image* m_logoImage{ nullptr }; // 非所有（マネージャが所有）
		bool m_hasChrome{ true };               // タイトルバー等の枠を付けているか
	};
} // namespace platform::window
