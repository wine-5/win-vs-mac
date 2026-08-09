#pragma once
#include "core/ecs/ComponentManager.h"
#include "core/ecs/Entity.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "core/interface/IUIRenderer.h"
#include "core/data/FileExtensionType.h"
#include "HudPanel.h"
#include <array>
#include <string>
#include <vector>

namespace game::data
{
	class FileEquipmentData; // 前方宣言
} // namespace game::data

namespace game::ui::ingame
{
	/**
	 * @brief Eキーで開く拡張子インベントリを描画するView
	 *
	 * 持っている拡張子と、今の能力値を並べて見せる。
	 * 拡張子だけを並べても「それを挿すと何がどうなるか」が分からないため、
	 * 能力値と同じ画面へ置いて突き合わせられるようにしている。
	 *
	 * 拡張子は出どころと状態で3つに分ける。
	 *   持ち込み   … セレクト画面で選んだもの（プレイ中は変えられない）
	 *   道中で拾った… 効果が乗っているもの
	 *   未装備     … 拾ったが枠が埋まっていて効果が乗っていないもの
	 * 「未装備」があることが、リネームブロックを探す動機になる。
	 *
	 * 開いている間は時間が止まる（PauseReason::Inventory）。読む画面なので、
	 * 読んでいる最中に殴られるのはプレイヤーの落ち度ではなく設計の落ち度になる。
	 */
	class InventoryView
	{
	  public:
		/**
		 * @brief InventoryViewのコンストラクタ
		 * @param uiRenderer UI描画のインターフェース
		 * @param screen 画面サイズ取得のインターフェース
		 * @param componentManager インベントリ・能力値の読み出しに使うComponentManagerの参照
		 * @param resourceManager アイコン画像の読み込みに使うリソース管理インターフェース
		 * @param equipmentData セレクト画面で選んだファイル（持ち込み）
		 */
		InventoryView(core::iface::IUIRenderer& uiRenderer,
		    core::iface::IScreen& screen,
		    core::ecs::ComponentManager& componentManager,
		    core::iface::IResourceManager& resourceManager,
		    const data::FileEquipmentData& equipmentData);

		/**
		 * @brief インベントリを描画する
		 * @param playerId プレイヤーのEntityID
		 */
		void draw(core::ecs::EntityId playerId);

	  private:
		/// @brief 能力値の項目数（左下HUD・セレクト画面と同じ8項目）
		static constexpr int STAT_COUNT{ 8 };

		/**
		 * @brief 1080p基準の長さを現在の画面サイズに合わせて変換する
		 * @param value 1080pでの長さ（ピクセル）
		 * @return 現在の画面高さに合わせた長さ（ピクセル）
		 */
		[[nodiscard]] int scaled(int value) const;

		/**
		 * @brief 見出しと拡張子アイコンの並びを1区分ぶん描く
		 *
		 * 横に並べきれない場合は折り返す。
		 * @param x 区分の左上X座標
		 * @param y 区分の左上Y座標
		 * @param width 使える幅
		 * @param caption 見出し（Shift_JIS変換済み）
		 * @param types 並べる拡張子種別
		 * @param isDimmed 効果が乗っていない扱いで淡く描くか
		 * @return 描画に使った高さ
		 */
		int drawSection(int x, int y, int width, const std::string& caption,
		    const std::vector<core::data::FileExtensionType>& types, bool isDimmed);

		/**
		 * @brief 拡張子アイコンを1つ描く
		 * @param x アイコン左上のX座標
		 * @param y アイコン左上のY座標
		 * @param type 拡張子種別（Count を渡すと空きスロットとして描く）
		 * @param isDimmed 淡く描くか
		 */
		void drawExtensionIcon(int x, int y, core::data::FileExtensionType type, bool isDimmed);

		/**
		 * @brief 能力値の一覧を描く
		 * @param x パネル左上のX座標
		 * @param y パネル左上のY座標
		 * @param width パネルの幅
		 * @param playerId プレイヤーのEntityID
		 */
		void drawStats(int x, int y, int width, core::ecs::EntityId playerId);

		core::iface::IUIRenderer& m_uiRenderer;
		core::iface::IScreen& m_screen;
		core::ecs::ComponentManager& m_componentManager;
		const data::FileEquipmentData& m_equipmentData;
		HudPanel m_panel;

		// 拡張子アイコン（FileExtensionTypeの並び順）と空きスロットのアイコン
		std::array<int, static_cast<int>(core::data::FileExtensionType::Count)> m_iconHandles{};
		int m_emptyIconHandle{ -1 };

		// 能力値アイコン（左下HUDと同じ並び）
		std::array<int, STAT_COUNT> m_statIconHandles{};

		// DxLibの描画はShift_JISを期待するため、日本語は生成時に一度だけ変換して持つ
		std::string m_captionCarried{};
		std::string m_captionAcquired{};
		std::string m_captionUnequipped{};
		std::string m_captionHint{};
		std::array<std::string, STAT_COUNT> m_statLabels{};
	};
} // namespace game::ui::ingame
