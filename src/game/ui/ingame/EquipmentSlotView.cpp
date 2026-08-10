#include "EquipmentSlotView.h"
#include "OrbitGlow.h"
#include "core/constant/UI.h"
#include "core/interface/IResourceManager.h"
#include "core/utility/Color.h"
#include "core/utility/Log.h"
#include "game/data/FileEquipmentData.h"
#include "game/constant/ExtensionIconId.h"
#include "game/utility/ExtensionBonusLabel.h"
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
	constexpr unsigned int SLOT_FILL_COLOR{ 0xFF0E1420 };
	constexpr int SLOT_FILL_ALPHA{ 184 }; // 約72%
	constexpr unsigned int SLOT_BORDER_COLOR{ 0xFF8CAAD2 };
	constexpr int SLOT_BORDER_ALPHA{ 46 };         // 約18%
	constexpr int SLOT_ACCENT_BORDER_ALPHA{ 200 }; // 装備済みスロットの枠

	constexpr const char* EMPTY_LABEL{ "--" };


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
	    core::iface::IResourceManager& resourceManager)
	    : m_uiRenderer{ uiRenderer }
	    , m_screen{ screen }
	    , m_equipmentData{ equipmentData }
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

	void EquipmentSlotView::draw()
	{
		constexpr int SLOT_COUNT{ data::FileEquipmentData::MAX_SLOTS };

		const int slotSize{ scaled(SLOT_SIZE) };
		const int gap{ scaled(SLOT_GAP) };
		const int totalWidth{ slotSize * SLOT_COUNT + gap * (SLOT_COUNT - 1) };

		// 右下アンカー。幅はモニタのアスペクト比で変わるため必ず実際の画面幅から逆算する
		const int startX{ m_screen.getWidth() - scaled(MARGIN) - totalWidth };
		const int y{ m_screen.getHeight() - scaled(MARGIN) - slotSize };

		for (int i{ 0 }; i < SLOT_COUNT; ++i)
		{
			const int x{ startX + i * (slotSize + gap) };
			const bool hasSelection{ m_equipmentData.hasSelection(i) };
			drawSlot(x, y, slotSize, m_equipmentData.getExtensionType(i), hasSelection);

			// 装備中のスロットだけ縁を光の粒が回り続ける（起動中であることの表現）
			if (hasSelection)
				drawOrbitingGlow(x, y, slotSize, i * orbit_glow::PHASE_PER_SLOT);
		}
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
