#include "InventoryView.h"
#include "core/base/ServiceLocator.h"
#include "core/constant/UI.h"
#include "core/interface/IStringConverter.h"
#include "core/utility/Color.h"
#include "core/utility/MathConstants.h"
#include "core/utility/Log.h"
#include "game/component/combat/AttackComponent.h"
#include "game/component/combat/ExtensionInventoryComponent.h"
#include "game/component/combat/HealthComponent.h"
#include "game/component/combat/PlayerStatsComponent.h"
#include "game/component/combat/PlayerStatBaseComponent.h"
#include "game/constant/ExtensionIconId.h"
#include "game/data/FileEquipmentData.h"
#include "game/utility/ExtensionBonusLabel.h"
#include <algorithm>
#include <cstdio>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// 画面全体を覆う暗幕。奥のゲーム画面を残したまま、手前の文字を読めるようにする
	constexpr int BACKDROP_ALPHA{ 176 };

	// 窓の大きさ（1080p基準）
	constexpr int WINDOW_WIDTH{ 1040 };
	constexpr int WINDOW_HEIGHT{ 600 };
	constexpr int WINDOW_PADDING{ 26 };

	// タイトルバー
	constexpr int TITLE_BAR_HEIGHT{ 52 };
	constexpr int TITLE_FONT_SIZE{ 22 };
	constexpr int TITLE_ICON_SIZE{ 22 };
	constexpr int TITLE_ICON_GAP{ 12 };

	// アドレスバー（パンくず）
	constexpr int ADDRESS_BAR_HEIGHT{ 44 };
	constexpr int ADDRESS_FONT_SIZE{ 17 };

	// ステータスバー
	constexpr int STATUS_BAR_HEIGHT{ 44 };
	constexpr int STATUS_FONT_SIZE{ 17 };

	// 区切り線と面の色
	constexpr unsigned int SEPARATOR_COLOR{ core::utility::Color::HUD_INK };
	constexpr int SEPARATOR_ALPHA{ 34 };

	// 左右の分割（左＝ファイル一覧、右＝パラメータ）
	constexpr int RIGHT_COLUMN_WIDTH{ 320 };

	// 一覧側の左右分割。左は枠数の決まっている区分（持ち込み・装備中）を縦に積み、
	// 右は数が決まらない所持一覧を置く。左だけに積むと右が丸ごと空いてしまう
	constexpr int SECTION_COLUMN_WIDTH{ 308 }; // スロット3つぶん（96*3 + 10*2）
	constexpr int SECTION_COLUMN_GAP{ 26 };

	// 所持一覧を囲む枠。区分の見出しだけだと、どこまでが所持一覧なのか境目が無い
	constexpr int PANE_PADDING{ 14 };
	constexpr int PANE_RADIUS{ 6 };
	constexpr int PANE_BORDER_ALPHA{ 40 };

	// 区分の見出し
	constexpr int SECTION_FONT_SIZE{ 18 };
	constexpr int SECTION_GAP{ 18 };        // 区分どうしの間隔
	constexpr int SECTION_CAPTION_GAP{ 8 }; // 見出しとマス目の間隔

	// マス目（スロット）
	constexpr int SLOT_WIDTH{ 96 };
	constexpr int SLOT_HEIGHT{ 138 }; // アイコン＋ファイル名＋ボーナス表記の3段ぶん
	constexpr int SLOT_GAP{ 10 };
	constexpr int SLOT_RADIUS{ 4 }; // Windows 11のコントロールの角丸
	constexpr int SLOT_ICON_SIZE{ 56 };
	constexpr int SLOT_ICON_TOP{ 10 };
	constexpr int SLOT_NAME_FONT_SIZE{ 14 };
	constexpr int SLOT_NAME_GAP{ 8 }; // アイコンとファイル名の間隔
	constexpr int SLOT_BONUS_FONT_SIZE{ 14 };
	constexpr int SLOT_BONUS_GAP{ 4 }; // ファイル名とボーナス表記の間隔

	constexpr unsigned int SLOT_FILL_COLOR{ 0xFF0E1420 };
	constexpr int SLOT_FILL_ALPHA{ 150 };
	constexpr unsigned int SLOT_BORDER_COLOR{ 0xFF8CAAD2 };
	constexpr int SLOT_BORDER_ALPHA{ 60 };
	constexpr int SLOT_EMPTY_BORDER_ALPHA{ 26 }; // 空きマスは枠だけ残して薄くする

	// 付け替え操作の選択（今いるマス）と掴んでいるマス。
	// 今いるマスは枠を強く、掴んでいるマスは面ごと染めて区別する。
	// 枠の強さだけで2種類を表そうとすると、どちらが動く側なのか読めない
	constexpr unsigned int CURSOR_COLOR{ core::utility::Color::HUD_ACCENT };
	constexpr int CURSOR_THICKNESS{ 3 };
	constexpr int CURSOR_MARGIN{ 3 }; // マスの外側へ広げる量。中身を隠さないため
	constexpr unsigned int HELD_COLOR{ core::utility::Color::HUD_CHARGE_MAX };
	constexpr int HELD_FILL_ALPHA{ 70 };

	constexpr int ICON_ALPHA_OPAQUE{ 255 }; // 効果が乗っているものはそのままの濃さで描く
	constexpr int EMPTY_ICON_ALPHA{ 60 };   // 空きマスの薄さ
	constexpr int DIMMED_ICON_ALPHA{ 140 }; // 効果が乗っていないものの薄さ

	// 能力値の並び
	constexpr int STAT_ROW_HEIGHT{ 44 };
	constexpr int STAT_ICON_SIZE{ 30 };
	constexpr int STAT_LABEL_GAP{ 10 };
	constexpr int STAT_FONT_SIZE{ 19 };

	// 強化されている項目の見せ方。左下HUDと同じ色を使い、
	// 「黄色＝強化されている」という意味を画面ごとにずらさない
	constexpr unsigned int STAT_BOOSTED_COLOR{ core::utility::Color::HUD_CHARGE_MAX };
	constexpr int STAT_DELTA_FONT_SIZE{ 16 };
	constexpr int STAT_DELTA_GAP{ 10 };         // 現在値と増分の間隔
	constexpr float STAT_BOOST_EPSILON{ 0.5f }; // これ未満の差は出さない（整数表示で0になるため）

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

	/**
	 * @brief 拡張子種別に対応する擬似ファイル名を返す
	 *
	 * 種別名（IMG・AUD）だけを並べると分類表に見えてしまう。
	 * ファイル名を添えることで「フォルダの中身を見ている」ことが伝わる
	 * @param type 拡張子種別
	 * @return 表示するファイル名
	 */
	const char* toFileName(core::data::FileExtensionType type)
	{
		switch (type)
		{
		case core::data::FileExtensionType::Executable: return "tool.exe";
		case core::data::FileExtensionType::Document: return "readme.txt";
		case core::data::FileExtensionType::Image: return "icon.png";
		case core::data::FileExtensionType::Audio: return "bgm.mp3";
		case core::data::FileExtensionType::SourceCode: return "main.cpp";
		case core::data::FileExtensionType::Shortcut: return "link.lnk";
		case core::data::FileExtensionType::Video: return "clip.mp4";
		case core::data::FileExtensionType::Archive: return "data.zip";
		default: return "unknown.dat";
		}
	}

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

			// 何をどれだけ上げるかを併記する。アイコンとファイル名だけでは
			// 「どれと入れ替えるべきか」を判断できない
			m_bonusLabels[i] = utility::ExtensionBonusLabel::format(
			    type, resourceManager.getExtensionBonus(type));
		}

		const std::string emptyIconId{ constant::extension_icon_id::EMPTY };
		m_emptyIconHandle = resourceManager.loadImageById(emptyIconId);

		for (int i{ 0 }; i < STAT_COUNT; ++i)
		{
			m_statIconHandles[i] = resourceManager.loadImageById(STAT_ICON_IMAGE_IDS[i]);
			m_statLabels[i] = toDrawable(STAT_LABELS[i]);
		}

		// 日本語は変換結果が毎フレーム同じなので、生成時に一度だけ変換して保持する
		m_title = toDrawable("インベントリ");
		m_addressText = toDrawable("PC  >  拡張子  >  所持しているもの");
		m_captionCarried = toDrawable("持ち込み（セレクト画面で選んだもの）");
		m_captionAcquired = toDrawable("道中で拾った（効果あり）");
		m_captionUnequipped = toDrawable("所持一覧（未装備・付け替え待ち）");
		m_captionNoUnequipped = toDrawable("拾ったものはすべて装備中です");
		m_captionStats = toDrawable("現在の能力");
		m_addressSwapText = toDrawable("PC  >  拡張子  >  付け替え");
		m_captionHint = toDrawable("E / Esc : 閉じる");

		// 付け替え中は操作が増える。どのキーで何ができるかを出しておかないと、
		// 掴んだあとで進み方が分からなくなる
		m_captionSwapHint = toDrawable("← → : 選ぶ    Enter : 入れ替え    F2 / Esc : 閉じる");
		m_captionEmptySlot = toDrawable("空き");
		m_captionOverflow = toDrawable(" 件は表示しきれません");
	}

	int InventoryView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	void InventoryView::setSelection(int cursorIndex, int heldIndex) noexcept
	{
		m_cursorIndex = cursorIndex;
		m_heldIndex = heldIndex;
	}

	void InventoryView::draw(core::ecs::EntityId playerId)
	{
		// 奥のゲーム画面を暗く落として、手前の文字を読めるようにする。
		// 真っ黒で覆わないのは「今どこに立っているか」を見失わせないため
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, BACKDROP_ALPHA);
		m_uiRenderer.drawBox(0, 0, m_screen.getWidth(), m_screen.getHeight(),
		    core::utility::Color::BLACK, true);
		m_uiRenderer.resetBlendMode();

		const int width{ scaled(WINDOW_WIDTH) };
		const int height{ scaled(WINDOW_HEIGHT) };
		const int left{ (m_screen.getWidth() - width) / 2 };
		const int top{ (m_screen.getHeight() - height) / 2 };

		// 光の帯は走らせない。小さなHUDでは生存確認として効くが、
		// この大きさだと白い帯が視界を横切って読む邪魔になる
		m_panel.draw(left, top, width, height, false);

		drawTitleBar(left, top, width);
		drawAddressBar(left, top + scaled(TITLE_BAR_HEIGHT), width);

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
		const int itemCount{ static_cast<int>(equipped.size() + unequipped.size()) };

		// 埋まっていない枠も空きマスとして見せる。何個挿せるかが分からないと
		// 「拾ってきて挿す」という行動につながらない
		while (equipped.size() < component::combat::ExtensionInventoryComponent::MAX_EQUIPPED)
			equipped.push_back(core::data::FileExtensionType::Count);

		const int padding{ scaled(WINDOW_PADDING) };
		const int rightWidth{ scaled(RIGHT_COLUMN_WIDTH) };
		const int contentTop{ top + scaled(TITLE_BAR_HEIGHT) + scaled(ADDRESS_BAR_HEIGHT) + padding };
		const int listWidth{ width - rightWidth - padding * 3 };

		// 一覧が使ってよい下限。ここを超えるとステータスバーへ重なる
		const int listBottom{ top + height - scaled(STATUS_BAR_HEIGHT) - padding };

		// 付け替えで動かせるのは道中で拾ったものだけ。持ち込みはセレクト画面で決めたもので、
		// 走っている最中には変えられないため選択の対象から外す
		constexpr int NOT_SELECTABLE{ -1 };
		const int unequippedBaseIndex{ component::combat::ExtensionInventoryComponent::MAX_EQUIPPED };

		// 左は枠数の決まっている区分。3つずつなので幅を固定し、余った右側を所持一覧へ渡す
		const int columnWidth{ scaled(SECTION_COLUMN_WIDTH) };
		const int columnGap{ scaled(SECTION_COLUMN_GAP) };

		int y{ contentTop };
		y += drawSection(left + padding, y, columnWidth, m_captionCarried, carried, false, listBottom,
		    NOT_SELECTABLE);
		y += scaled(SECTION_GAP);
		drawSection(left + padding, y, columnWidth, m_captionAcquired, equipped, false, listBottom, 0);

		drawHoldingPane(left + padding + columnWidth + columnGap, contentTop,
		    listWidth - columnWidth - columnGap, listBottom, unequipped, unequippedBaseIndex);

		// 左右の区切り線。エクスプローラーのペイン分割に相当する
		const int dividerX{ left + width - rightWidth - padding * 2 };
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, SEPARATOR_ALPHA);
		m_uiRenderer.drawLine(dividerX, contentTop - padding / 2,
		    dividerX, top + height - scaled(STATUS_BAR_HEIGHT) - padding / 2, SEPARATOR_COLOR, 1);
		m_uiRenderer.resetBlendMode();

		drawStats(dividerX + padding, contentTop, rightWidth, playerId);
		drawStatusBar(left, top + height - scaled(STATUS_BAR_HEIGHT), width, itemCount);
	}

	void InventoryView::drawTitleBar(int x, int y, int width)
	{
		// 面の色は変えない。上下で濃さが違うと、窓が2枚重なっているように見える。
		// 区切りは下端の線だけで足りる
		const int barHeight{ scaled(TITLE_BAR_HEIGHT) };
		const int padding{ scaled(WINDOW_PADDING) };
		const int iconSize{ scaled(TITLE_ICON_SIZE) };

		// フォルダを表す四角。専用の画像を持たずに済ませ、色だけで「フォルダ」を示す
		m_uiRenderer.drawRoundedBox(x + padding, y + (barHeight - iconSize) / 2,
		    iconSize, iconSize, scaled(SLOT_RADIUS), core::utility::Color::HUD_CHARGE_MAX, true, 1);

		const int fontSize{ scaled(TITLE_FONT_SIZE) };
		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		m_uiRenderer.drawText(x + padding + iconSize + scaled(TITLE_ICON_GAP),
		    y + (barHeight - fontSize) / 2, m_title.c_str(),
		    core::utility::Color::HUD_INK, fontSize);
		m_uiRenderer.resetFont();

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, SEPARATOR_ALPHA);
		m_uiRenderer.drawLine(x, y + barHeight, x + width, y + barHeight, SEPARATOR_COLOR, 1);
		m_uiRenderer.resetBlendMode();
	}

	void InventoryView::drawAddressBar(int x, int y, int width)
	{
		const int barHeight{ scaled(ADDRESS_BAR_HEIGHT) };
		const int fontSize{ scaled(ADDRESS_FONT_SIZE) };

		// 付け替え中は行き先を変える。同じ窓でも今やっていることが違うと示す
		const std::string& address{ isSwapMode() ? m_addressSwapText : m_addressText };

		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		m_uiRenderer.drawText(x + scaled(WINDOW_PADDING), y + (barHeight - fontSize) / 2,
		    address.c_str(), core::utility::Color::HUD_INK_FAINT, fontSize);
		m_uiRenderer.resetFont();

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, SEPARATOR_ALPHA);
		m_uiRenderer.drawLine(x, y + barHeight, x + width, y + barHeight, SEPARATOR_COLOR, 1);
		m_uiRenderer.resetBlendMode();
	}

	void InventoryView::drawStatusBar(int x, int y, int width, int itemCount)
	{
		const int barHeight{ scaled(STATUS_BAR_HEIGHT) };
		const int fontSize{ scaled(STATUS_FONT_SIZE) };
		const int padding{ scaled(WINDOW_PADDING) };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, SEPARATOR_ALPHA);
		m_uiRenderer.drawLine(x, y, x + width, y, SEPARATOR_COLOR, 1);
		m_uiRenderer.resetBlendMode();

		char countText[32]{};
		std::snprintf(countText, sizeof(countText), "%d", itemCount);
		const std::string countLabel{ std::string(countText) + toDrawable(" 個の項目") };

		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		m_uiRenderer.drawText(x + padding, y + (barHeight - fontSize) / 2,
		    countLabel.c_str(), core::utility::Color::HUD_INK_FAINT, fontSize);

		// 操作の案内。開いたはいいが閉じ方が分からない、を起こさない
		const std::string& hint{ isSwapMode() ? m_captionSwapHint : m_captionHint };
		const int hintWidth{ m_uiRenderer.getTextWidth(hint.c_str(), fontSize) };
		m_uiRenderer.drawText(x + width - padding - hintWidth, y + (barHeight - fontSize) / 2,
		    hint.c_str(), core::utility::Color::HUD_INK_FAINT, fontSize);
		m_uiRenderer.resetFont();
	}

	void InventoryView::drawHoldingPane(int x, int y, int width, int maxBottom,
	    const std::vector<core::data::FileExtensionType>& types, int baseIndex)
	{
		const int pad{ scaled(PANE_PADDING) };

		// 枠は先に描く。あとから重ねるとマス目や文字の上に線が乗る
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, PANE_BORDER_ALPHA);
		m_uiRenderer.drawRoundedBox(x - pad, y - pad, width + pad * 2,
		    maxBottom - y + pad * 2, scaled(PANE_RADIUS), SLOT_BORDER_COLOR, false, 1);
		m_uiRenderer.resetBlendMode();

		if (types.empty())
		{
			// 空でも見出しは残す。枠だけがあって何も書いていないと、
			// 表示が壊れているのか中身が無いのかを区別できない
			const int captionFontSize{ scaled(SECTION_FONT_SIZE) };
			m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
			m_uiRenderer.drawText(x, y, m_captionUnequipped.c_str(),
			    core::utility::Color::HUD_INK_FAINT, captionFontSize);
			m_uiRenderer.drawText(x, y + captionFontSize + scaled(SECTION_CAPTION_GAP),
			    m_captionNoUnequipped.c_str(), core::utility::Color::HUD_INK_FAINT, captionFontSize);
			m_uiRenderer.resetFont();
			return;
		}

		drawSection(x, y, width, m_captionUnequipped, types, true, maxBottom - pad, baseIndex);
	}

	int InventoryView::drawSection(int x, int y, int width, const std::string& caption,
	    const std::vector<core::data::FileExtensionType>& types, bool isDimmed,
	    int maxBottom, int selectableBaseIndex)
	{
		const int captionFontSize{ scaled(SECTION_FONT_SIZE) };
		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		m_uiRenderer.drawText(x, y, caption.c_str(),
		    core::utility::Color::HUD_INK_FAINT, captionFontSize);
		m_uiRenderer.resetFont();

		const int slotWidth{ scaled(SLOT_WIDTH) };
		const int slotHeight{ scaled(SLOT_HEIGHT) };
		const int slotGap{ scaled(SLOT_GAP) };
		const int slotTop{ y + captionFontSize + scaled(SECTION_CAPTION_GAP) };

		// 横に並べきれなくなったら折り返す。所持数に上限を設けていないため、
		// 1行に収まる前提で書くと拾い集めたときに窓からはみ出す
		const int perRow{ std::max(1, (width + slotGap) / (slotWidth + slotGap)) };

		// 窓に収まる行数を先に出す。はみ出したぶんを黙って描くと
		// ステータスバーの上へ重なって読めなくなる
		const int available{ maxBottom - slotTop };
		const int maxRows{ std::max(0, (available + slotGap) / (slotHeight + slotGap)) };
		const int totalRows{ (static_cast<int>(types.size()) + perRow - 1) / perRow };
		const int drawnRows{ std::min(totalRows, maxRows) };

		const std::size_t drawnCount{ static_cast<std::size_t>(drawnRows) * perRow };
		for (std::size_t i{ 0 }; i < types.size() && i < drawnCount; ++i)
		{
			const int column{ static_cast<int>(i) % perRow };
			const int row{ static_cast<int>(i) / perRow };

			const bool isSelectable{ selectableBaseIndex >= 0 };
			const int acquiredIndex{ isSelectable ? selectableBaseIndex + static_cast<int>(i) : -1 };

			drawSlot(x + column * (slotWidth + slotGap),
			    slotTop + row * (slotHeight + slotGap), types[i], isDimmed,
			    isSelectable && acquiredIndex == m_cursorIndex,
			    isSelectable && acquiredIndex == m_heldIndex);
		}

		int usedHeight{ slotTop - y + drawnRows * (slotHeight + slotGap) - slotGap };
		if (drawnRows <= 0)
			usedHeight = slotTop - y;

		// 収まらなかったぶんは件数だけ示す。黙って消すと「拾ったはずのものが無い」
		// と受け取られてしまう
		if (types.size() > drawnCount)
		{
			char overflowText[64]{};
			std::snprintf(overflowText, sizeof(overflowText), "+ %d",
			    static_cast<int>(types.size() - drawnCount));
			const std::string label{ std::string(overflowText) + m_captionOverflow };

			m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
			m_uiRenderer.drawText(x, y + usedHeight + scaled(SECTION_CAPTION_GAP), label.c_str(),
			    core::utility::Color::HUD_INK_FAINT, scaled(SECTION_FONT_SIZE));
			m_uiRenderer.resetFont();
			usedHeight += scaled(SECTION_CAPTION_GAP) + scaled(SECTION_FONT_SIZE);
		}
		return usedHeight;
	}

	void InventoryView::drawSlot(int x, int y, core::data::FileExtensionType type, bool isDimmed,
	    bool isCursor, bool isHeld)
	{
		const bool isEmpty{ type == core::data::FileExtensionType::Count };
		const int slotWidth{ scaled(SLOT_WIDTH) };
		const int slotHeight{ scaled(SLOT_HEIGHT) };
		const int radius{ scaled(SLOT_RADIUS) };

		// 掴んでいるマスは面ごと染める。枠だけだと選択位置と見分けが付かず、
		// どちらが動く側なのか分からなくなる
		if (isHeld)
		{
			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, HELD_FILL_ALPHA);
			m_uiRenderer.drawRoundedBox(x, y, slotWidth, slotHeight, radius, HELD_COLOR, true, 1);
			m_uiRenderer.resetBlendMode();
		}

		// マス目の面と枠。これがあることで「置き場」に見え、
		// アイコンが宙に浮いている状態から抜け出す
		if (!isEmpty)
		{
			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, SLOT_FILL_ALPHA);
			m_uiRenderer.drawRoundedBox(x, y, slotWidth, slotHeight, radius, SLOT_FILL_COLOR, true, 1);
			m_uiRenderer.resetBlendMode();
		}

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
		    isEmpty ? SLOT_EMPTY_BORDER_ALPHA : SLOT_BORDER_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, slotWidth, slotHeight, radius, SLOT_BORDER_COLOR, false, 1);
		m_uiRenderer.resetBlendMode();

		const int handle{ isEmpty ? m_emptyIconHandle : m_iconHandles[static_cast<int>(type)] };
		const int iconSize{ scaled(SLOT_ICON_SIZE) };
		if (handle != -1)
		{
			int alpha{ ICON_ALPHA_OPAQUE };
			if (isEmpty)
				alpha = EMPTY_ICON_ALPHA;
			else if (isDimmed)
				alpha = DIMMED_ICON_ALPHA;

			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, alpha);
			m_uiRenderer.drawImage(handle, x + (slotWidth - iconSize) / 2,
			    y + scaled(SLOT_ICON_TOP), iconSize, iconSize);
			m_uiRenderer.resetBlendMode();
		}

		// ファイル名。種別名だけを並べると分類表に見えるため、名前を添えて
		// 「フォルダの中身を見ている」ことを伝える
		const int nameFontSize{ scaled(SLOT_NAME_FONT_SIZE) };
		const std::string name{ isEmpty ? m_captionEmptySlot : std::string(toFileName(type)) };
		const int nameY{ y + scaled(SLOT_ICON_TOP) + iconSize + scaled(SLOT_NAME_GAP) };

		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		const int nameWidth{ m_uiRenderer.getTextWidth(name.c_str(), nameFontSize) };
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
		    isEmpty ? EMPTY_ICON_ALPHA : ICON_ALPHA_OPAQUE);
		m_uiRenderer.drawText(x + (slotWidth - nameWidth) / 2, nameY, name.c_str(),
		    isDimmed ? core::utility::Color::HUD_INK_FAINT : core::utility::Color::HUD_INK,
		    nameFontSize);
		m_uiRenderer.resetBlendMode();

		// 何をどれだけ上げるか。これが無いと、どれを挿すべきかを
		// 覚えているかどうかの勝負になってしまう
		if (!isEmpty)
		{
			const std::string& bonus{ m_bonusLabels[static_cast<int>(type)] };
			const int bonusFontSize{ scaled(SLOT_BONUS_FONT_SIZE) };
			const int bonusWidth{ m_uiRenderer.getTextWidth(bonus.c_str(), bonusFontSize) };

			m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA,
			    isDimmed ? DIMMED_ICON_ALPHA : ICON_ALPHA_OPAQUE);
			m_uiRenderer.drawText(x + (slotWidth - bonusWidth) / 2,
			    nameY + nameFontSize + scaled(SLOT_BONUS_GAP), bonus.c_str(),
			    STAT_BOOSTED_COLOR, bonusFontSize);
		}
		m_uiRenderer.resetBlendMode();
		m_uiRenderer.resetFont();

		// 選択位置の枠は最後に、マスの外側へ描く。中身の上へ重ねると
		// アイコンやボーナス表記が枠に食われて読めなくなる
		if (isCursor)
		{
			const int margin{ scaled(CURSOR_MARGIN) };
			m_uiRenderer.drawRoundedBox(x - margin, y - margin,
			    slotWidth + margin * 2, slotHeight + margin * 2,
			    radius + margin, CURSOR_COLOR, false, scaled(CURSOR_THICKNESS));
		}
	}

	void InventoryView::drawStats(int x, int y, int width, core::ecs::EntityId playerId)
	{
		std::array<float, STAT_COUNT> stats{};
		if (const auto* attack{ m_componentManager.tryGet<component::combat::AttackComponent>(playerId) })
		{
			stats[STAT_INDEX_ATK] = attack->m_attackPower;
			stats[STAT_INDEX_RNG] = attack->m_attackRange;
			stats[STAT_INDEX_CRIT] = attack->m_criticalRate * core::utility::RATIO_TO_PERCENT; // 割合を百分率へ
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

		// 強化前の値。現在値と突き合わせて「どれだけ上がっているか」を出す。
		// 数値だけでは、それが素の値なのか拡張子で伸びたものなのか分からない
		std::array<float, STAT_COUNT> baseStats{};
		if (const auto* base{ m_componentManager.tryGet<component::combat::PlayerStatBaseComponent>(playerId) })
		{
			baseStats[STAT_INDEX_HP] = base->m_maxHp;
			baseStats[STAT_INDEX_ATK] = base->m_attackPower;
			baseStats[STAT_INDEX_DEF] = base->m_defence;
			baseStats[STAT_INDEX_SPD] = base->m_moveSpeed;
			baseStats[STAT_INDEX_RNG] = base->m_attackRange;
			baseStats[STAT_INDEX_CRIT] = base->m_criticalRate * core::utility::RATIO_TO_PERCENT;
			baseStats[STAT_INDEX_BSPD] = base->m_projectileSpeed;
			baseStats[STAT_INDEX_BRNG] = base->m_projectileRange;
		}

		const int captionFontSize{ scaled(SECTION_FONT_SIZE) };
		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		m_uiRenderer.drawText(x, y, m_captionStats.c_str(),
		    core::utility::Color::HUD_INK_FAINT, captionFontSize);
		m_uiRenderer.resetFont();

		const int iconSize{ scaled(STAT_ICON_SIZE) };
		const int rowHeight{ scaled(STAT_ROW_HEIGHT) };
		const int fontSize{ scaled(STAT_FONT_SIZE) };

		int rowY{ y + captionFontSize + scaled(SECTION_CAPTION_GAP) };
		for (int i{ 0 }; i < STAT_COUNT; ++i)
		{
			if (m_statIconHandles[i] != -1)
				m_uiRenderer.drawImage(m_statIconHandles[i], x,
				    rowY + (rowHeight - iconSize) / 2, iconSize, iconSize);

			m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
			m_uiRenderer.drawText(x + iconSize + scaled(STAT_LABEL_GAP),
			    rowY + (rowHeight - fontSize) / 2, m_statLabels[i].c_str(),
			    core::utility::Color::HUD_INK_FAINT, fontSize);
			m_uiRenderer.resetFont();

			char valueText[32]{};
			if (i == STAT_INDEX_CRIT)
				std::snprintf(valueText, sizeof(valueText), "%d%%", static_cast<int>(stats[i]));
			else
				std::snprintf(valueText, sizeof(valueText), "%d", static_cast<int>(stats[i]));

			// 素の値を上回っていれば強化中として色を変える。
			// 出どころ（装備ファイル・拾った拡張子）は問わない
			const float delta{ stats[i] - baseStats[i] };
			const bool isBoosted{ delta >= STAT_BOOST_EPSILON };

			// 数値は右寄せ。桁が動いても右端が揃い、増えたことを見比べやすい
			m_uiRenderer.setFont(core::constant::ui::MONO_FONT_NAME);
			const int valueWidth{ m_uiRenderer.getTextWidth(valueText, fontSize) };
			m_uiRenderer.drawText(x + width - valueWidth,
			    rowY + (rowHeight - fontSize) / 2, valueText,
			    isBoosted ? STAT_BOOSTED_COLOR : core::utility::Color::HUD_INK, fontSize);

			// 増分は現在値の左へ添える。いくつ伸びたかが分かると、
			// どの拡張子が効いているのかを結び付けられる
			if (isBoosted)
			{
				char deltaText[32]{};
				if (i == STAT_INDEX_CRIT)
					std::snprintf(deltaText, sizeof(deltaText), "+%d%%", static_cast<int>(delta));
				else
					std::snprintf(deltaText, sizeof(deltaText), "+%d", static_cast<int>(delta));

				const int deltaFontSize{ scaled(STAT_DELTA_FONT_SIZE) };
				const int deltaWidth{ m_uiRenderer.getTextWidth(deltaText, deltaFontSize) };
				m_uiRenderer.drawText(x + width - valueWidth - scaled(STAT_DELTA_GAP) - deltaWidth,
				    rowY + (rowHeight - deltaFontSize) / 2, deltaText,
				    STAT_BOOSTED_COLOR, deltaFontSize);
			}
			m_uiRenderer.resetFont();

			rowY += rowHeight;
		}
	}
} // namespace game::ui::ingame
