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
	 * 本物のタイトルバー・最小化・最大化・閉じるボタンを持つが、操作には一切反応しない
	 * （閉じる・ドラッグ・フォーカス奪取をすべて無効化）。
	 * オーナー付きウィンドウのためタスクバーには出ず、常に最前面でゲーム内の弾に追従する。
	 * クライアント領域にはロゴ画像を全面描画する。小さくなるとタイトルバーを外し、
	 * ロゴが最後まで見えるようにする。
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
		 * @brief タイトルバー左上のアプリケーションアイコンを設定する
		 * @param icon アイコンハンドル（所有権は呼び出し側＝マネージャが持つ）
		 */
		void setTitleIcon(HICON icon) noexcept;

		/**
		 * @brief フェードアウトを開始する（現在のアルファから0へ向かう）
		 */
		void startFadeOut() noexcept;

		/**
		 * @brief フェードを進める（フェード中のみアルファを減らす）
		 * @param deltaTime フレーム間の時間差
		 */
		void updateFade(float deltaTime) noexcept;

		/** @brief フェードアウト中か（完了して非表示になったらfalseに戻る） */
		[[nodiscard]] bool isFading() const noexcept
		{
			return m_isFading;
		}

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

		/**
		 * @brief 現在のm_alphaをウィンドウへ適用する（レイヤードアルファ）
		 */
		void applyAlpha() noexcept;

		Gdiplus::Image* m_logoImage{ nullptr }; // 非所有（マネージャが所有）
		bool m_hasChrome{ true };               // タイトルバー等の枠を付けているか
		float m_alpha{ 255.0f };                // 現在のアルファ（0〜255）
		bool m_isFading{ false };               // フェードアウト中か
	};
} // namespace platform::window
