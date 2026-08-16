#pragma once
#include "core/data/GameSettings.h"
#include "core/interface/IScreen.h"
#include "core/interface/IUIRenderer.h"
#include <string>

namespace game::ui::settings
{
	/**
	 * @brief 設定画面の左ナビの項目（＝ページ）
	 */
	enum class SettingsPage
	{
		Sound,
		Control,
	};

	/** @brief 左ナビの項目数 */
	inline constexpr int PAGE_COUNT{ 2 };

	/**
	 * @brief サウンドページの行（上から並ぶ順。選択位置はこの番号で表す）
	 */
	enum class SoundRow
	{
		Master,
		Bgm,
		Se,
		Reset,
		Count,
	};

	/**
	 * @brief 操作ページの行（上から並ぶ順）
	 */
	enum class ControlRow
	{
		Sensitivity,
		InvertY,
		Shake,
		Reset,
		Count,
	};

	/**
	 * @brief 行の右側に置くコントロールの種類
	 */
	enum class ControlKind
	{
		Slider,
		Toggle,
		Button,
	};

	/**
	 * @brief 指定ページの行数を返す
	 * @param page 対象のページ
	 * @return 行数
	 */
	[[nodiscard]] constexpr int getRowCount(SettingsPage page) noexcept
	{
		return page == SettingsPage::Sound
		           ? static_cast<int>(SoundRow::Count)
		           : static_cast<int>(ControlRow::Count);
	}

	/**
	 * @brief 指定した行のコントロールの種類を返す
	 * @param page 対象のページ
	 * @param row ページ内の行番号
	 * @return コントロールの種類（範囲外なら Button）
	 */
	[[nodiscard]] ControlKind getControlKind(SettingsPage page, int row) noexcept;

	/**
	 * @brief 設定画面（Windows 11「設定」アプリ風）を描画するクラス
	 *
	 * 描画と、マウス判定のための位置の問い合わせを担当する。状態は持たない。
	 * どこを押したかの判定を Controller 側で計算し直すと、見えている行と反応する行が
	 * ずれる余地が生まれるため、座標を知っているこちらが答える
	 */
	class SettingsPanelView
	{
	  public:
		/**
		 * @brief SettingsPanelViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 */
		SettingsPanelView(core::iface::IUIRenderer& uiRenderer, core::iface::IScreen& screen);

		/**
		 * @brief 設定画面を描画する
		 * @param settings 表示する設定値
		 * @param page 表示中のページ
		 * @param focusIndex 選択位置（0〜PAGE_COUNT-1 は左ナビ、それ以降がページ内の行）
		 * @param showFocus フォーカス枠を出すか（キーボード・パッド操作中だけ true）
		 */
		void draw(const core::data::GameSettings& settings, SettingsPage page,
		    int focusIndex, bool showFocus);

		/**
		 * @brief 指定座標にある選択位置を返す
		 * @param page 表示中のページ
		 * @param x マウスのX座標
		 * @param y マウスのY座標
		 * @return 選択位置（左ナビまたは行）。どこでもなければ -1
		 */
		[[nodiscard]] int getFocusIndexAt(SettingsPage page, int x, int y);

		/**
		 * @brief 指定座標がウィンドウの外かを返す
		 * @param x マウスのX座標
		 * @param y マウスのY座標
		 * @return ウィンドウの外なら true
		 */
		[[nodiscard]] bool isOutsideWindow(SettingsPage page, int x, int y);

		/**
		 * @brief 指定座標が閉じるボタン（×）の上かを返す
		 * @param page 表示中のページ
		 * @param x マウスのX座標
		 * @param y マウスのY座標
		 * @return 閉じるボタンの上なら true
		 */
		[[nodiscard]] bool isOnCloseButton(SettingsPage page, int x, int y);

		/**
		 * @brief 指定座標がその行のスライダーの上かを返す
		 * @param page 表示中のページ
		 * @param row ページ内の行番号
		 * @param x マウスのX座標
		 * @param y マウスのY座標
		 * @return スライダーの上なら true
		 */
		[[nodiscard]] bool isOnSlider(SettingsPage page, int row, int x, int y);

		/**
		 * @brief X座標をスライダー上の割合へ変換する
		 * @param page 表示中のページ
		 * @param x マウスのX座標
		 * @return 0.0〜1.0 の割合（左端が0.0）
		 */
		[[nodiscard]] float getSliderRatioAt(SettingsPage page, int x);

	  private:
		/** @brief 1ページに置けるセクションの上限 */
		static constexpr int MAX_SECTIONS{ 3 };
		/** @brief 1ページに置ける行の上限 */
		static constexpr int MAX_ROWS{ 4 };

		void drawWindow() const;
		void drawTitleBar() const;
		void drawNav(SettingsPage page, int focusIndex, bool showFocus) const;
		void drawPage(const core::data::GameSettings& settings, SettingsPage page,
		    int selectedRow, bool showFocus) const;

		/**
		 * @brief 選択中を示す下地と、キーボード操作中だけ出す二重のフォーカス枠を描く
		 * @param x 左上X座標
		 * @param y 左上Y座標
		 * @param width 幅
		 * @param height 高さ
		 * @param showFocus フォーカス枠を出すか
		 */
		void drawSelection(int x, int y, int width, int height, bool showFocus) const;

		/**
		 * @brief 指定した行数ぶんのカードの枠と区切り線を描く
		 * @param y カードの上端Y
		 * @param rowCount カードに収める行数
		 */
		void drawCard(int y, int rowCount) const;

		/**
		 * @brief カード内の1行を描く
		 * @param y 行の上端Y
		 * @param title 行の見出し（UTF-8）
		 * @param sub 行の説明（UTF-8。空文字なら描かない）
		 * @param kind 右側に置くコントロールの種類
		 * @param value 値（スライダーは現在値、トグルは0/1、ボタンは未使用）
		 * @param minValue スライダーの下限
		 * @param maxValue スライダーの上限
		 * @param isSelected 選択中の行か
		 * @param showFocus フォーカス枠を出すか
		 */
		void drawRow(int y, const char* title, const char* sub, ControlKind kind,
		    int value, int minValue, int maxValue, bool isSelected, bool showFocus) const;

		void drawSlider(int centerY, int value, int minValue, int maxValue) const;
		void drawToggle(int centerY, bool isOn) const;
		void drawResetButton(int centerY) const;

		/**
		 * @brief 左ナビの項目の矩形を返す
		 * @param index 項目番号
		 * @param outX 左上X座標の出力先
		 * @param outY 左上Y座標の出力先
		 * @param outWidth 幅の出力先
		 * @param outHeight 高さの出力先
		 */
		void getNavItemRect(int index, int& outX, int& outY, int& outWidth, int& outHeight) const;

		/**
		 * @brief UTF-8の文字列を描画用（Shift-JIS）へ変換する
		 * @param utf8 変換する文字列
		 * @return 変換後の文字列
		 */
		[[nodiscard]] std::string toDrawable(const char* utf8) const;

		/**
		 * @brief 基準サイズ（高さ660px）での値を、いまの画面サイズへ換算する
		 * @param basePixels 基準サイズでのピクセル数
		 * @return 換算後のピクセル数（最低1）
		 */
		[[nodiscard]] int scaled(float basePixels) const noexcept;

		/**
		 * @brief ウィンドウの位置・大きさと、ページ内の見出し・カード・行のY座標を求め直す
		 *
		 * 描画も当たり判定もここが出した座標だけを見る。積み上げ計算が2か所にあると、
		 * 片方だけ直したときに「見えている行と反応する行が違う」というずれ方をする
		 * @param page 対象のページ
		 */
		void updateLayout(SettingsPage page);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;

		// updateLayout() が求めるレイアウト（すべてピクセル）
		int m_panelX{};
		int m_panelY{};
		int m_panelWidth{};
		int m_panelHeight{};
		int m_navWidth{};
		int m_titleBarHeight{};
		int m_contentX{};
		int m_contentTop{};
		int m_contentWidth{};
		int m_rowHeight{};
		float m_scale{ 1.0f }; // 基準サイズ（高さ660px）に対する倍率

		int m_sectionLabelY[MAX_SECTIONS]{}; // 見出しの上端Y
		int m_cardY[MAX_SECTIONS]{};         // カードの上端Y
		int m_rowY[MAX_ROWS]{};              // 各行の上端Y

		// スライダーは全行で同じ位置・同じ幅に置くため、行ごとではなくまとめて持つ
		int m_sliderLeft{};
		int m_sliderWidth{};
	};
} // namespace game::ui::settings
