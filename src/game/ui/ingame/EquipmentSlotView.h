#pragma once
#include "core/data/FileExtensionType.h"
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IUIRenderer.h"
#include "core/interface/IScreen.h"
#include <array>
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace core::iface
{
	class IResourceManager; // 前方宣言
} // namespace core::iface

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
		 * @param resourceManager 拡張子アイコンの読み込みに使うIResourceManager
		 * @param componentManager 道中で拾った拡張子を読むComponentManagerの参照
		 * @param playerId プレイヤーのEntityID
		 */
		EquipmentSlotView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    const data::FileEquipmentData& equipmentData,
		    core::iface::IResourceManager& resourceManager,
		    core::ecs::ComponentManager& componentManager,
		    core::ecs::EntityId playerId);

		/**
		 * @brief 装備スロットを描画する
		 */
		void draw();

	  private:
		/// @brief マスの縁が表す意味。色で役割を見分けさせる
		enum class SlotAccent
		{
			Normal, // 道中で拾って装備しているもの・空き枠
			Locked, // セレクト画面で選んだ枠（道中では変えられない）
			Gained  // RAMブロックで増えた枠
		};

		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		/**
		 * @brief どちらのページを見ているかの見出しを描く
		 * @param x スロットの並びの左端X座標
		 * @param y 見出し上端のY座標
		 * @param width スロットの並びの幅
		 * @param page 表示中のページ
		 */
		void drawPageLabel(int x, int y, int width, int page);

		/**
		 * @brief スロット1枠を描画する
		 * @param x スロット左上のX座標
		 * @param y スロット左上のY座標
		 * @param size スロットの一辺の長さ
		 * @param type 装備中の拡張子種別
		 * @param hasSelection 装備済みかどうか（falseなら空きスロットとして描く）
		 * @param accent 縁の意味づけ
		 */
		void drawSlot(int x, int y, int size, core::data::FileExtensionType type,
		    bool hasSelection, SlotAccent accent);

		/**
		 * @brief 指定範囲の中央にテキストを描画する
		 * @param centerX 中央のX座標
		 * @param y テキスト上端のY座標
		 * @param text 描画する文字列
		 * @param color 色（ARGB形式：0xAARRGGBB）
		 * @param fontSize フォントサイズ
		 */
		void drawCenteredText(int centerX, int y, const char* text, unsigned int color, int fontSize);

		/**
		 * @brief 拡張子種別に対応するアイコンの画像ハンドルを取得する
		 * @param type 拡張子種別
		 * @return 画像ハンドル（読み込めていない場合は-1）
		 */
		[[nodiscard]] int getIconHandle(core::data::FileExtensionType type) const;

		/**
		 * @brief スロットの縁に沿って光の粒を周回させる
		 *
		 * 静止したHUDは死んで見えるため、装備中のスロットだけを常に動かして
		 * 「起動中のプログラム」であることを示す。装備していないスロットは動かさない
		 * @param x スロット左上のX座標
		 * @param y スロット左上のY座標
		 * @param size スロットの一辺の長さ
		 * @param phaseOffset 周回位相のずらし量（0.0〜1.0。スロットごとに変えて同期させない）
		 */
		void drawOrbitingGlow(int x, int y, int size, float phaseOffset);

		/**
		 * @brief 道中で拾って効果が乗っている拡張子を集める
		 *
		 * 効果が乗るのは先頭の m_maxEquipped 個だけなので、そのぶんだけを返す。
		 * 枠が埋まっていない位置は「空き」として Count を入れ、
		 * あと何個挿せるのかが枠の数で分かるようにする
		 * @return 表示する拡張子種別の並び（要素数は持ち込みの枠数と同じ）
		 */
		[[nodiscard]] std::vector<core::data::FileExtensionType> collectAcquired() const;

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		const data::FileEquipmentData& m_equipmentData;
		core::ecs::ComponentManager& m_componentManager;
		core::ecs::EntityId m_playerId;

		// 拡張子種別ごとのアイコン画像ハンドル。生成時に一度だけ読み込む
		std::unordered_map<int, int> m_iconHandles{};
		// 空きスロットに描くアイコンの画像ハンドル
		int m_emptyIconHandle{ -1 };

		// ページの見出し（Shift_JIS変換済み。添字はページ番号）
		std::array<std::string, 2> m_pageLabels{};

		// 周回演出の基準時刻。描画経路からしか呼ばれずdeltaTimeを受け取らないため、
		// 経過時間は壁時計から求める
		std::chrono::steady_clock::time_point m_startTime{ std::chrono::steady_clock::now() };
	};
} // namespace game::ui::ingame
