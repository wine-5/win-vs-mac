#include "InventoryView.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/UI.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include "core/utility/Log.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/ExtensionInventoryComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/combat/PlayerStatsComponent.h"
#include "game/constant/ExtensionIconId.h"
#include "game/data/FileEquipmentData.h"
#include <algorithm>
#include <cstdio>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// 画面全体を覆う暗幕。奥のゲーム画面を残したまま、手前の文字を読めるようにする
	constexpr int BACKDROP_ALPHA{ 168 };

	// 2枚のパネルの配置（1080p基準）。左に拡張子、右に能力値
	constexpr int PANEL_GAP{ 24 };
	constexpr int LEFT_PANEL_WIDTH{ 620 };
	constexpr int RIGHT_PANEL_WIDTH{ 380 };
	constexpr int PANEL_HEIGHT{ 520 };
	constexpr int PANEL_PADDING{ 28 };

	// 見出し
	constexpr int TITLE_FONT_SIZE{ 24 };
	constexpr int TITLE_Y{ 20 };
	constexpr int SECTION_FONT_SIZE{ 19 };
	constexpr int SECTION_GAP{ 22 }; // 区分どうしの間隔

	// 拡張子アイコンの並び
	constexpr int ICON_SIZE{ 84 };
	constexpr int ICON_GAP{ 12 };
	constexpr int ICON_CAPTION_GAP{ 10 };   // 見出しとアイコンの間隔
	constexpr int ICON_ALPHA_OPAQUE{ 255 }; // 効果が乗っているものはそのままの濃さで描く
	constexpr int EMPTY_ICON_ALPHA{ 90 };   // 空きスロットの薄さ
	constexpr int DIMMED_ICON_ALPHA{ 130 }; // 効果が乗っていないものの薄さ

	// 能力値の並び
	constexpr int STAT_ROW_HEIGHT{ 50 };
	constexpr int STAT_ICON_SIZE{ 34 };
	constexpr int STAT_LABEL_GAP{ 12 };
	constexpr int STAT_FONT_SIZE{ 20 };

	// 閉じ方の案内
	constexpr int HINT_FONT_SIZE{ 18 };
	constexpr int HINT_BOTTOM_MARGIN{ 24 };

	// 能力値アイコンの画像ID（左下HUD・セレクト画面と同じ並び）
	constexpr std::array<const char*, 8> STAT_ICON_IMAGE_IDS{
		"stat-hp", "stat-atk", "stat-def", "stat-spd",
		"stat-rng", "stat-crit", "stat-bspd", "stat-brng"
	};

	// 能力値のラベル（UTF-8。生成時にShift_JISへ変換して持つ）
	constexpr std::array<const char*, 8> STAT_LABELS{
		"HP", "攻撃", "防御", "速度", "射程", "会心", "弾速", "飛距離"
	};

	// STAT_ICON_IMAGE_IDS 上の位置。値を詰める側と並びがずれないよう名前で参照する
	constexpr int STAT_INDEX_HP{ 0 };
	constexpr int STAT_INDEX_ATK{ 1 };
	constexpr int STAT_INDEX_DEF{ 2 };
	constexpr int STAT_INDEX_SPD{ 3 };
	constexpr int STAT_INDEX_RNG{ 4 };
	constexpr int STAT_INDEX_CRIT{ 5 }; // 会心だけは割合なので百分率で見せる
	constexpr int STAT_INDEX_BSPD{ 6 };
	constexpr int STAT_INDEX_BRNG{ 7 };

	constexpr const char* MONO_FONT_NAME{ "Cascadia Mono" };
	constexpr const char* UI_FONT_NAME{ "Noto Sans JP" };

	constexpr const char* TITLE_TEXT{ "INVENTORY" };

	/**
	 * @brief UTF-8の文字列をDxLibが期待するShift_JISへ変換する
	 * @param text UTF-8の文字列
	 * @return 変換後の文字列（変換器が無ければそのまま）
	 */
	std::string toDrawable(const char* text)
	{
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		return converter ? converter->utf8ToShiftJis(text) : std::string{ text };
	}
} // namespace

namespace game::ui::ingame
{
	InventoryView::InventoryView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    core::ecs::ComponentManager& componentManager,
	    core::iface::IResourceManager& resourceManager,
	    const data::FileEquipmentData& equipmentData)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_componentManager{ componentManager }
	    , m_equipmentData{ equipmentData }
	    , m_panel{ uiRenderer, screen }
	{
		// アイコンは毎フレーム引き直さず、生成時に一度だけ読み込む
		for (int i{ 0 }; i < static_cast<int>(core::data::FileExtensionType::Count); ++i)
		{
			const auto type{ static_cast<core::data::FileExtensionType>(i) };
			const std::string imageId{ constant::toExtensionIconId(type) };
			m_iconHandles[i] = resourceManager.loadImageById(imageId);
			if (m_iconHandles[i] == -1)
				core::log::error("インベントリの拡張子アイコン '{}' の読み込みに失敗しました", imageId.c_str());
		}

		const std::string emptyIconId{ constant::extension_icon_id::EMPTY };
		m_emptyIconHandle = resourceManager.loadImageById(emptyIconId);

		for (int i{ 0 }; i < STAT_COUNT; ++i)
		{
			m_statIconHandles[i] = resourceManager.loadImageById(STAT_ICON_IMAGE_IDS[i]);
			m_statLabels[i] = toDrawable(STAT_LABELS[i]);
		}

		// 日本語は変換結果が毎フレーム同じなので、生成時に一度だけ変換して保持する
		m_captionCarried = toDrawable("持ち込み");
		m_captionAcquired = toDrawable("道中で拾った");
		m_captionUnequipped = toDrawable("未装備（付け替え待ち）");
		m_captionHint = toDrawable("E : 閉じる");
	}

	int InventoryView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void InventoryView::draw(core::ecs::EntityId playerId)
	{
		// 奥のゲーム画面を暗く落として、手前の文字を読めるようにする。
		// 真っ黒で覆わないのは「今どこに立っているか」を見失わせないため
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, BACKDROP_ALPHA);
		m_uiRenderer.drawBox(0, 0, m_screen.getWidth(), m_screen.getHeight(),
		    core::utility::Color::BLACK, true);
		m_uiRenderer.resetBlendMode();

		const int leftWidth{ scaled(LEFT_PANEL_WIDTH) };
		const int rightWidth{ scaled(RIGHT_PANEL_WIDTH) };
		const int height{ scaled(PANEL_HEIGHT) };
		const int gap{ scaled(PANEL_GAP) };

		const int totalWidth{ leftWidth + gap + rightWidth };
		const int left{ (m_screen.getWidth() - totalWidth) / 2 };
		const int top{ (m_screen.getHeight() - height) / 2 };

		m_panel.draw(left, top, leftWidth, height);
		m_panel.draw(left + leftWidth + gap, top, rightWidth, height);

		const int padding{ scaled(PANEL_PADDING) };
		m_uiRenderer.setFont(UI_FONT_NAME);
		m_uiRenderer.drawText(left + padding, top + scaled(TITLE_Y), TITLE_TEXT,
		    core::utility::Color::HUD_INK, scaled(TITLE_FONT_SIZE));
		m_uiRenderer.resetFont();

		// 持ち込み（セレクト画面で選んだもの）
		std::vector<core::data::FileExtensionType> carried{};
		for (int i{ 0 }; i < data::FileEquipmentData::MAX_SLOTS; ++i)
		{
			carried.push_back(m_equipmentData.hasSelection(i)
			                      ? m_equipmentData.getExtensionType(i)
			                      : core::data::FileExtensionType::Count);
		}

		// 道中で拾ったもの。先頭が装備中、それ以降が未装備
		std::vector<core::data::FileExtensionType> equipped{};
		std::vector<core::data::FileExtensionType> unequipped{};
		if (const auto* inventory{ m_componentManager.tryGet<component::combat::ExtensionInventoryComponent>(playerId) })
		{
			for (std::size_t i{ 0 }; i < inventory->m_acquired.size(); ++i)
			{
				if (inventory->isEquipped(static_cast<int>(i)))
					equipped.push_back(inventory->m_acquired[i]);
				else
					unequipped.push_back(inventory->m_acquired[i]);
			}
		}

		// 埋まっていない枠も空きとして見せる。何個挿せるかが分からないと
		// 「拾ってきて挿す」という行動につながらない
		while (equipped.size() < component::combat::ExtensionInventoryComponent::MAX_EQUIPPED)
			equipped.push_back(core::data::FileExtensionType::Count);

		const int contentX{ left + padding };
		const int contentWidth{ leftWidth - padding * 2 };
		int y{ top + scaled(TITLE_Y) + scaled(TITLE_FONT_SIZE) + scaled(SECTION_GAP) };

		y += drawSection(contentX, y, contentWidth, m_captionCarried, carried, false);
		y += scaled(SECTION_GAP);
		y += drawSection(contentX, y, contentWidth, m_captionAcquired, equipped, false);
		y += scaled(SECTION_GAP);
		if (!unequipped.empty())
			drawSection(contentX, y, contentWidth, m_captionUnequipped, unequipped, true);

		drawStats(left + leftWidth + gap, top, rightWidth, playerId);

		// 閉じ方の案内。開いたはいいが閉じ方が分からない、を起こさない
		m_uiRenderer.setFont(UI_FONT_NAME);
		const int hintWidth{ m_uiRenderer.getTextWidth(m_captionHint.c_str(), scaled(HINT_FONT_SIZE)) };
		m_uiRenderer.drawText((m_screen.getWidth() - hintWidth) / 2,
		    top + height + scaled(HINT_BOTTOM_MARGIN), m_captionHint.c_str(),
		    core::utility::Color::HUD_INK_FAINT, scaled(HINT_FONT_SIZE));
		m_uiRenderer.resetFont();
	}

	int InventoryView::drawSection(int x, int y, int width, const std::string& caption,
	    const std::vector<core::data::FileExtensionType>& types, bool isDimmed)
	{
		m_uiRenderer.setFont(UI_FONT_NAME);
		m_uiRenderer.drawText(x, y, caption.c_str(),
		    core::utility::Color::HUD_INK_FAINT, scaled(SECTION_FONT_SIZE));
		m_uiRenderer.resetFont();

		const int iconSize{ scaled(ICON_SIZE) };
		const int iconGap{ scaled(ICON_GAP) };
		const int iconTop{ y + scaled(SECTION_FONT_SIZE) + scaled(ICON_CAPTION_GAP) };

		// 横に並べきれなくなったら折り返す。所持数に上限を設けていないため、
		// 1行に収まる前提で書くと拾い集めたときに画面外へはみ出す
		const int perRow{ std::max(1, (width + iconGap) / (iconSize + iconGap)) };

		int row{ 0 };
		for (std::size_t i{ 0 }; i < types.size(); ++i)
		{
			const int column{ static_cast<int>(i) % perRow };
			row = static_cast<int>(i) / perRow;
			drawExtensionIcon(x + column * (iconSize + iconGap),
			    iconTop + row * (iconSize + iconGap), types[i], isDimmed);
		}

		const int rowCount{ types.empty() ? 0 : row + 1 };
		return iconTop - y + rowCount * (iconSize + iconGap) - iconGap;
	}

	void InventoryView::drawExtensionIcon(int x, int y, core::data::FileExtensionType type, bool isDimmed)
	{
		const bool isEmpty{ type == core::data::FileExtensionType::Count };
		const int handle{ isEmpty ? m_emptyIconHandle : m_iconHandles[static_cast<int>(type)] };
		if (handle == -1)
			return;

		const int size{ scaled(ICON_SIZE) };
		int alpha{ ICON_ALPHA_OPAQUE };
		if (isEmpty)
			alpha = EMPTY_ICON_ALPHA;
		else if (isDimmed)
			alpha = DIMMED_ICON_ALPHA;

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);
		m_uiRenderer.drawImage(handle, x, y, size, size);
		m_uiRenderer.resetBlendMode();
	}

	void InventoryView::drawStats(int x, int y, int width, core::ecs::EntityId playerId)
	{
		std::array<float, STAT_COUNT> stats{};
		if (const auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(playerId) })
		{
			stats[STAT_INDEX_ATK] = attack->m_attackPower;
			stats[STAT_INDEX_RNG] = attack->m_attackRange;
			stats[STAT_INDEX_CRIT] = attack->m_criticalRate * 100.0f; // 割合を百分率へ
		}
		if (const auto* health{ m_componentManager.tryGet<component::combat::HealthComponent>(playerId) })
		{
			stats[STAT_INDEX_HP] = health->m_maxHp;
			stats[STAT_INDEX_DEF] = health->m_defence;
		}
		if (const auto* player{ m_componentManager.tryGet<component::combat::PlayerStatsComponent>(playerId) })
		{
			stats[STAT_INDEX_SPD] = player->m_moveSpeed;
			stats[STAT_INDEX_BSPD] = player->m_projectileSpeed;
			stats[STAT_INDEX_BRNG] = player->m_projectileRange;
		}

		const int padding{ scaled(PANEL_PADDING) };
		const int iconSize{ scaled(STAT_ICON_SIZE) };
		const int rowHeight{ scaled(STAT_ROW_HEIGHT) };
		const int fontSize{ scaled(STAT_FONT_SIZE) };

		int rowY{ y + scaled(TITLE_Y) };
		for (int i{ 0 }; i < STAT_COUNT; ++i)
		{
			if (m_statIconHandles[i] != -1)
				m_uiRenderer.drawImage(m_statIconHandles[i], x + padding,
				    rowY + (rowHeight - iconSize) / 2, iconSize, iconSize);

			m_uiRenderer.setFont(UI_FONT_NAME);
			m_uiRenderer.drawText(x + padding + iconSize + scaled(STAT_LABEL_GAP),
			    rowY + (rowHeight - fontSize) / 2, m_statLabels[i].c_str(),
			    core::utility::Color::HUD_INK_FAINT, fontSize);
			m_uiRenderer.resetFont();

			char valueText[32]{};
			if (i == STAT_INDEX_CRIT)
				std::snprintf(valueText, sizeof(valueText), "%d%%", static_cast<int>(stats[i]));
			else
				std::snprintf(valueText, sizeof(valueText), "%d", static_cast<int>(stats[i]));

			// 数値は右寄せ。桁が動いても右端が揃い、増えたことを見比べやすい
			m_uiRenderer.setFont(MONO_FONT_NAME);
			const int valueWidth{ m_uiRenderer.getTextWidth(valueText, fontSize) };
			m_uiRenderer.drawText(x + width - padding - valueWidth,
			    rowY + (rowHeight - fontSize) / 2, valueText,
			    core::utility::Color::HUD_INK, fontSize);
			m_uiRenderer.resetFont();

			rowY += rowHeight;
		}
	}
} // namespace game::ui::ingame
