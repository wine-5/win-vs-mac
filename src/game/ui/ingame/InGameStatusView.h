#pragma once
#include "core/data/Difficulty.h"
#include "core/interface/IScreen.h"
#include "core/interface/IUIRenderer.h"
#include "HudPanel.h"
#include <string>

namespace core::iface
{
	class IStringConverter; // 前方宣言
} // namespace core::iface

namespace game::ui::ingame
{
	/**
	 * @brief インゲーム右上の状況表示（難易度・経過時間）を描画するView
	 *
	 * 難易度はプレイ中に確かめる手段が無く、Hardで敵が強いのか自分が弱いのか判別できない。
	 * 経過時間もリザルトまで分からないと、急ぐべきか立て直すべきかを判断できない。
	 * この2つを常時見える位置に置いて、プレイ中の判断材料にする。
	 *
	 * デザインはWindows 11（Fluent）に準拠し、角丸8pxの半透明パネルに載せる。
	 * レイアウトの数値はすべて1080p基準で持ち、画面高さに応じて拡大縮小する
	 */
	class InGameStatusView
	{
	  public:
		/**
		 * @brief InGameStatusViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param difficulty 表示する難易度（プレイ中は変わらないので生成時に受け取る）
		 */
		InGameStatusView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::data::Difficulty difficulty);

		/**
		 * @brief 状況を描画する
		 * @param elapsedTime インゲーム開始からの経過時間（秒）
		 */
		void draw(float elapsedTime);

	  private:
		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		HudPanel m_panel;

		core::data::Difficulty m_difficulty{ core::data::Difficulty::Normal };

		// DxLibの描画はShift_JISを期待するため、ソース上のUTF-8日本語をそのまま渡すと文字化けする。
		// 変換結果は毎フレーム同じなので生成時に一度だけ変換して保持する
		std::string m_captionText{};
	};
} // namespace game::ui::ingame
