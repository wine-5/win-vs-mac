#include "EquipmentSlotView.h"
#include "OrbitGlow.h"
#include "core/constant/UI.h"
#include "core/interface/IResourceManager.h"
#include "core/utility/Color.h"
#include "core/utility/Log.h"
#include "game/data/FileEquipmentData.h"
#include "game/component/combat/ExtensionInventoryComponent.h"
#include "game/constant/ExtensionIconId.h"
#include "game/utility/ExtensionBonusLabel.h"
#include "core/base/ServiceLocator.h"
#include "core/interface/IStringConverter.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// スロットの配置（右下・1080p基準）
	constexpr int MARGIN{ 28 };
	constexpr int SLOT_SIZE{ 116 };
	constexpr int SLOT_GAP{ 16 };
	constexpr int SLOT_RADIUS{ 4 }; // Windows 11のボタン・コントロールの角丸

	// スロット内の要素位置（スロット左上からの相対座標・1080p基準）
	constexpr int ICON_Y{ 12 };
	constexpr int ICON_SIZE{ 68 };
	constexpr int TYPE_LABEL_Y{ 30 }; // アイコンの読み込みに失敗したときの代替表示に使う
	constexpr int TYPE_FONT_SIZE{ 26 };
	constexpr int BONUS_LABEL_Y{ 84 };
	constexpr int BONUS_FONT_SIZE{ 19 };

	// スロットの塗りと枠。色と不透明度を分けて持つ（DxLibのブレンドはアルファを別途指定するため）
	constexpr unsigned int SLOT_FILL_COLOR{ core::utility::Color::HUD_PANEL_FILL };
	constexpr int SLOT_FILL_ALPHA{ 184 }; // 約72%
	constexpr unsigned int SLOT_BORDER_COLOR{ core::utility::Color::HUD_PANEL_BORDER };
	constexpr int SLOT_BORDER_ALPHA{ 46 };         // 約18%
	constexpr int SLOT_ACCENT_BORDER_ALPHA{ 200 }; // 装備済みスロットの枠

	constexpr const char* EMPTY_LABEL{ "--" };

	// 表示するページ。3枠しか置けないため、持ち込みと道中で拾ったぶんを交互に見せる。
	// 枠を6つ並べると視界の右下がふさがり、戦闘中に見えない場所が増える
	constexpr int PAGE_CARRIED{ 0 };  // セレクト画面で選んだもの
	constexpr int PAGE_ACQUIRED{ 1 }; // 道中で拾ったもの
	constexpr int PAGE_COUNT{ 2 };
	constexpr float PAGE_INTERVAL{ 5.0f }; // 自動で切り替わる間隔（秒。左下HUDと揃える）

	// どちらのページを見ているかの見出し。これが無いと、切り替わった瞬間に
	// 「装備が勝手に変わった」と読めてしまう
	constexpr int PAGE_LABEL_FONT_SIZE{ 18 };
	constexpr int PAGE_LABEL_GAP{ 8 }; // 見出しとスロットの間隔

	// 見出しの下敷き。HUDの背後はステージの絵で、明るい床の上では
	// 文字色をどう選んでも沈む。スロットと同じ暗い面を敷いて読めるようにする
	constexpr int PAGE_LABEL_PADDING_X{ 10 };
	constexpr int PAGE_LABEL_PADDING_Y{ 4 };

	// 切り替えは下から浮き上がらせる。瞬間的に絵が入れ替わると、
	// 切り替わったのか元から違ったのかが分からない
	constexpr float SLIDE_DURATION{ 0.30f }; // 切り替えアニメの長さ（秒）
	constexpr int SLIDE_OFFSET{ 12 };        // スライドの振れ幅（1080p基準）

	/**
	 * @brief 拡張子種別の表示名を返す
	 * @param type 拡張子種別
	 * @return スロットに表示する短い名前
	 */
	const char* toTypeLabel(core::data::FileExtensionType type)
	{
		switch (type)
		{
		case core::data::FileExtensionType::Executable: return "EXE";
		case core::data::FileExtensionType::Document: return "DOC";
		case core::data::FileExtensionType::Image: return "IMG";
		case core::data::FileExtensionType::Audio: return "AUD";
		case core::data::FileExtensionType::SourceCode: return "SRC";
		case core::data::FileExtensionType::Shortcut: return "LNK";
		case core::data::FileExtensionType::Video: return "VID";
		case core::data::FileExtensionType::Archive: return "ARC";
		default: return "ETC";
		}
	}


} // namespace

namespace game::ui::ingame
{
	EquipmentSlotView::EquipmentSlotView(core::iface::IUIRenderer& uiRenderer,
	    core::iface::IScreen& screen,
	    const data::FileEquipmentData& equipmentData,
	    core::iface::IResourceManager& resourceManager,
	    core::ecs::ComponentManager& componentManager,
	    core::ecs::EntityId playerId)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_equipmentData{ equipmentData }
	    , m_componentManager{ componentManager }
	    , m_playerId{ playerId }
	{
		// アイコンは毎フレーム引き直さず、生成時に一度だけ読み込む。
		// 失敗しても描画は続けられる（文字表示へ退避する）ため、記録に留める
		for (int i{ 0 }; i < static_cast<int>(core::data::FileExtensionType::Count); ++i)
		{
			const auto type{ static_cast<core::data::FileExtensionType>(i) };
			const std::string imageId{ constant::toExtensionIconId(type) };
			const int handle{ resourceManager.loadImageById(imageId) };
			if (handle == -1)
				core::log::error("装備スロットの拡張子アイコン '{}' の読み込みに失敗しました", imageId.c_str());
			m_iconHandles[i] = handle;
		}

		const std::string emptyIconId{ constant::extension_icon_id::EMPTY };
		m_emptyIconHandle = resourceManager.loadImageById(emptyIconId);
		if (m_emptyIconHandle == -1)
			core::log::error("装備スロットの空きアイコン '{}' の読み込みに失敗しました", emptyIconId.c_str());

		// DxLibの描画はShift_JISを期待する。毎フレーム同じ結果なので生成時に一度だけ変換する
		auto* converter{ core::base::ServiceLocator::get<core::iface::IStringConverter>() };
		m_pageLabels[PAGE_CARRIED] = converter ? converter->utf8ToShiftJis("セレクト画面で選択") : "セレクト画面で選択";
		m_pageLabels[PAGE_ACQUIRED] = converter ? converter->utf8ToShiftJis("道中で取得") : "道中で取得";
	}

	int EquipmentSlotView::getIconHandle(core::data::FileExtensionType type) const
	{
		const auto it{ m_iconHandles.find(static_cast<int>(type)) };
		return it == m_iconHandles.end() ? -1 : it->second;
	}

	int EquipmentSlotView::scaled(int value) const
	{
		return value * m_screen.getHeight() / BASE_SCREEN_HEIGHT;
	}

	std::vector<core::data::FileExtensionType> EquipmentSlotView::collectAcquired() const
	{
		const auto* inventory{
			m_componentManager.tryGet<component::combat::ExtensionInventoryComponent>(m_playerId)
		};
		if (inventory == nullptr)
			return {};

		// 枠の数はRAMブロックで増えるので、持ち込み側の3つ固定とは分けて数える。
		// 埋まっていない位置は空きとして残し、あと何個挿せるかを枠の数で示す
		std::vector<core::data::FileExtensionType> acquired(
		    inventory->m_maxEquipped, core::data::FileExtensionType::Count);

		const int count{ inventory->equippedCount() };
		for (int i{ 0 }; i < count && i < inventory->m_maxEquipped; ++i)
			acquired[i] = inventory->m_acquired[i];
		return acquired;
	}

	void EquipmentSlotView::draw()
	{
		const auto acquired{ collectAcquired() };

		// 何も拾っていないうちはページを送らない。空の枠へ切り替わっても
		// 見るものが無く、持ち込みを確認したいときに邪魔になるだけ
		const bool hasAcquired{ std::any_of(acquired.begin(), acquired.end(),
			[](core::data::FileExtensionType type)
			{ return type != core::data::FileExtensionType::Count; }) };

		const float elapsed{ std::chrono::duration<float>(
			std::chrono::steady_clock::now() - m_startTime)
			    .count() };
		const int page{ hasAcquired
			                ? static_cast<int>(elapsed / PAGE_INTERVAL) % PAGE_COUNT
			                : 0 };

		// ページが変わった直後だけ下から持ち上げる。送っていないときは動かさない
		const float sincePageChange{ std::fmod(elapsed, PAGE_INTERVAL) };
		const float slideProgress{ hasAcquired
			                           ? std::min(sincePageChange / SLIDE_DURATION, 1.0f)
			                           : 1.0f };
		const int slideOffset{ static_cast<int>(scaled(SLIDE_OFFSET) * (1.0f - slideProgress)) };

		// 表示する枠の数はページで変わる。持ち込みは3つ固定、
		// 道中で拾ったぶんは増えた枠の数だけ並ぶ
		const int slotCount{ page == PAGE_CARRIED
			                     ? data::FileEquipmentData::MAX_SLOTS
			                     : static_cast<int>(acquired.size()) };

		const int slotSize{ scaled(SLOT_SIZE) };
		const int gap{ scaled(SLOT_GAP) };
		const int totalWidth{ slotSize * slotCount + gap * (slotCount - 1) };

		// 右下アンカー。幅はモニタのアスペクト比で変わるため必ず実際の画面幅から逆算する
		const int startX{ m_screen.getWidth() - scaled(MARGIN) - totalWidth };
		const int y{ m_screen.getHeight() - scaled(MARGIN) - slotSize + slideOffset };

		drawPageLabel(startX,
		    y - scaled(PAGE_LABEL_GAP) - scaled(PAGE_LABEL_PADDING_Y) - scaled(PAGE_LABEL_FONT_SIZE),
		    totalWidth, page);

		for (int i{ 0 }; i < slotCount; ++i)
		{
			const int x{ startX + i * (slotSize + gap) };

			const bool isCarriedPage{ page == PAGE_CARRIED };
			const auto type{ isCarriedPage ? m_equipmentData.getExtensionType(i) : acquired[i] };
			const bool hasSelection{ isCarriedPage
				                         ? m_equipmentData.hasSelection(i)
				                         : type != core::data::FileExtensionType::Count };

			drawSlot(x, y, slotSize, type, hasSelection);

			// 装備中のスロットだけ縁を光の粒が回り続ける（起動中であることの表現）
			if (hasSelection)
				drawOrbitingGlow(x, y, slotSize, i * orbit_glow::PHASE_PER_SLOT);
		}
	}

	void EquipmentSlotView::drawPageLabel(int x, int y, int width, int page)
	{
		const int fontSize{ scaled(PAGE_LABEL_FONT_SIZE) };
		const int paddingX{ scaled(PAGE_LABEL_PADDING_X) };
		const int paddingY{ scaled(PAGE_LABEL_PADDING_Y) };

		m_uiRenderer.setFont(core::constant::ui::UI_FONT_NAME);
		const int textWidth{ m_uiRenderer.getTextWidth(m_pageLabels[page].c_str(), fontSize) };

		// スロットの並びと右端を揃える。左寄せだと枠の幅が変わったときにずれる
		const int textX{ x + width - textWidth };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, SLOT_FILL_ALPHA);
		m_uiRenderer.drawRoundedBox(textX - paddingX, y - paddingY,
		    textWidth + paddingX * 2, fontSize + paddingY * 2, scaled(SLOT_RADIUS),
		    SLOT_FILL_COLOR, true, 1);
		m_uiRenderer.resetBlendMode();

		m_uiRenderer.drawText(textX, y, m_pageLabels[page].c_str(),
		    core::utility::Color::HUD_INK, fontSize);
		m_uiRenderer.resetFont();
	}

	void EquipmentSlotView::drawSlot(int x, int y, int size, core::data::FileExtensionType type, bool hasSelection)
	{
		const int radius{ scaled(SLOT_RADIUS) };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, SLOT_FILL_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, size, size, radius, SLOT_FILL_COLOR, true, 1);

		// 装備済みはアクセント色の枠で締める。空きは枠を薄いままにして視線を集めない
		const unsigned int borderColor{ hasSelection ? core::utility::Color::HUD_ACCENT : SLOT_BORDER_COLOR };
		const int borderAlpha{ hasSelection ? SLOT_ACCENT_BORDER_ALPHA : SLOT_BORDER_ALPHA };
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, borderAlpha);
		m_uiRenderer.drawRoundedBox(x, y, size, size, radius, borderColor, false, 1);
		m_uiRenderer.resetBlendMode();

		const int centerX{ x + size / 2 };
		const int iconSize{ scaled(ICON_SIZE) };
		const int iconX{ centerX - iconSize / 2 };
		const int iconY{ y + scaled(ICON_Y) };

		m_uiRenderer.setFont(core::constant::ui::MONO_FONT_NAME);

		if (!hasSelection)
		{
			// 空きスロットは点線枠のアイコンで示す。画像が無ければ文字で代替する
			if (m_emptyIconHandle != -1)
				m_uiRenderer.drawImage(m_emptyIconHandle, iconX, iconY, iconSize, iconSize);
			else
				drawCenteredText(centerX, y + scaled(TYPE_LABEL_Y), EMPTY_LABEL,
				    core::utility::Color::HUD_INK_FAINT, scaled(TYPE_FONT_SIZE));
			m_uiRenderer.resetFont();
			return;
		}

		// 全種別にアイコンがある。読み込みに失敗したときだけ種別名を文字で見せる
		const int iconHandle{ getIconHandle(type) };
		if (iconHandle != -1)
			m_uiRenderer.drawImage(iconHandle, iconX, iconY, iconSize, iconSize);
		else
			drawCenteredText(centerX, y + scaled(TYPE_LABEL_Y), toTypeLabel(type),
			    core::utility::Color::HUD_INK, scaled(TYPE_FONT_SIZE));

		// 何を強化するかを併記する。装備の効果は開始時のステータス補正としてのみ
		// 現れるため、種別名だけでは何の役に立っているのか分からない
		const std::string bonusLabel{ utility::ExtensionBonusLabel::toLabel(type) };
		drawCenteredText(centerX, y + scaled(BONUS_LABEL_Y), bonusLabel.c_str(),
		    core::utility::Color::HUD_INK_FAINT, scaled(BONUS_FONT_SIZE));

		m_uiRenderer.resetFont();
	}

	void EquipmentSlotView::drawOrbitingGlow(int x, int y, int size, float phaseOffset)
	{
		const float elapsed{ std::chrono::duration<float>(
			std::chrono::steady_clock::now() - m_startTime)
			    .count() };

		orbit_glow::draw(m_uiRenderer, x, y, size, size, elapsed, phaseOffset,
		    scaled(orbit_glow::DOT_RADIUS));
	}

	void EquipmentSlotView::drawCenteredText(int centerX, int y, const char* text, unsigned int color, int fontSize)
	{
		const int width{ m_uiRenderer.getTextWidth(text, fontSize) };
		m_uiRenderer.drawText(centerX - width / 2, y, text, color, fontSize);
	}
} // namespace game::ui::ingame
