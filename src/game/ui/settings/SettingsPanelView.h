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
	 * @brief 設定画面（Windows 11「設定」アプリ風）を描画するクラス
	 *
	 * 描画だけを担当し、入力も状態も持たない。どこから開いても同じ見た目になるよう、
	 * タイトル・ポーズのどちらから開いてもこのクラスが描く
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
		 * @param selectedRow 選択中の行（0起点。範囲外なら選択なし）
		 * @param showFocus フォーカス枠を出すか（キーボード・パッド操作中だけ true）
		 */
		void draw(const core::data::GameSettings& settings, SettingsPage page,
		    int selectedRow, bool showFocus);

	  private:
		/** @brief 行の右側に置くコントロールの種類 */
		enum class ControlKind
		{
			Slider,
			Toggle,
			Button,
		};

		void drawWindow() const;
		void drawTitleBar() const;
		void drawNav(SettingsPage page) const;
		void drawSoundPage(const core::data::AudioSettings& audio, int selectedRow, bool showFocus) const;
		void drawControlPage(const core::data::ControlSettings& control, int selectedRow, bool showFocus) const;

		/**
		 * @brief セクションの見出しを描き、続くカードの上端Yを返す
		 * @param y 見出しの上端Y
		 * @param label 見出しの文字列（UTF-8）
		 * @return カードの上端Y
		 */
		[[nodiscard]] int drawSectionLabel(int y, const char* label) const;

		/**
		 * @brief 指定した行数ぶんのカードを描き、1行目の上端Yを返す
		 * @param y カードの上端Y
		 * @param rowCount カードに収める行数
		 * @return 1行目の上端Y
		 */
		[[nodiscard]] int drawCard(int y, int rowCount) const;

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

		void drawSlider(int right, int centerY, int value, int minValue, int maxValue) const;
		void drawToggle(int right, int centerY, bool isOn) const;
		void drawResetButton(int right, int centerY) const;

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
		 * @brief 画面サイズからウィンドウの位置と大きさを求め直す
		 *
		 * 解像度が変わっても崩れないよう、描画のたびに計算する
		 */
		void updateLayout();

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
	};
} // namespace game::ui::settings
