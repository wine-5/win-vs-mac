#pragma once
#include "core/data/FileExtensionType.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"

namespace game::data
{
	class FileEquipmentData; // 前方宣言
} // namespace game::data

namespace game::ui::ingame
{
	/**
	 * @brief インゲーム右下の装備スロット（セレクト画面で選んだファイル）を描画するView
	 *
	 * デザインはWindows 11（Fluent）に準拠し、スロットは角丸4pxのコントロールとして描く。
	 * 装備は開始時にステータス補正として適用されるだけで実行中に切り替わらないため、
	 * 「今どの拡張子を装備しているか」と「それが何を強化しているか」を常時表示する。
	 *
	 * レイアウトの数値はすべて1080p基準で持ち、画面高さに応じて拡大縮小する
	 */
	class EquipmentSlotView
	{
	  public:
		/**
		 * @brief EquipmentSlotViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param equipmentData 装備中のファイル情報（所有はGameManager）
		 */
		EquipmentSlotView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    const data::FileEquipmentData& equipmentData);

		/**
		 * @brief 装備スロットを描画する
		 */
		void draw();

	  private:
		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		/**
		 * @brief スロット1枠を描画する
		 * @param x スロット左上のX座標
		 * @param y スロット左上のY座標
		 * @param size スロットの一辺の長さ
		 * @param type 装備中の拡張子種別
		 * @param hasSelection 装備済みかどうか（falseなら空きスロットとして描く）
		 */
		void drawSlot(int x, int y, int size, core::data::FileExtensionType type, bool hasSelection);

		/**
		 * @brief 指定範囲の中央にテキストを描画する
		 * @param centerX 中央のX座標
		 * @param y テキスト上端のY座標
		 * @param text 描画する文字列
		 * @param color 色（ARGB形式：0xAARRGGBB）
		 * @param fontSize フォントサイズ
		 */
		void drawCenteredText(int centerX, int y, const char* text, unsigned int color, int fontSize);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		const data::FileEquipmentData& m_equipmentData;
	};
} // namespace game::ui::ingame
