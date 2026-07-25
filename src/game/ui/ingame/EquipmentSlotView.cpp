#include "EquipmentSlotView.h"
#include "core/constant/UI.h"
#include "core/interface/IResourceManager.h"
#include "core/utility/Color.h"
#include "core/utility/Log.h"
#include "game/data/FileEquipmentData.h"
#include <array>
#include <utility>

namespace
{
	// 基準解像度。レイアウトの数値はすべてこの高さのときのピクセル数として書く
	constexpr int BASE_SCREEN_HEIGHT{ 1080 };

	// スロットの配置（右下・1080p基準）
	constexpr int MARGIN{ 28 };
	constexpr int SLOT_SIZE{ 92 };
	constexpr int SLOT_GAP{ 16 };
	constexpr int SLOT_RADIUS{ 4 }; // Windows 11のボタン・コントロールの角丸

	// スロット内の要素位置（スロット左上からの相対座標・1080p基準）
	constexpr int ICON_Y{ 10 };
	constexpr int ICON_SIZE{ 48 };
	constexpr int TYPE_LABEL_Y{ 24 }; // アイコンが無いとき（Unknown）の代替表示に使う
	constexpr int TYPE_FONT_SIZE{ 22 };
	constexpr int BONUS_LABEL_Y{ 62 };
	constexpr int BONUS_FONT_SIZE{ 14 };

	// スロットの塗りと枠。色と不透明度を分けて持つ（DxLibのブレンドはアルファを別途指定するため）
	constexpr unsigned int SLOT_FILL_COLOR{ 0xFF0E1420 };
	constexpr int SLOT_FILL_ALPHA{ 184 }; // 約72%
	constexpr unsigned int SLOT_BORDER_COLOR{ 0xFF8CAAD2 };
	constexpr int SLOT_BORDER_ALPHA{ 46 };         // 約18%
	constexpr int SLOT_ACCENT_BORDER_ALPHA{ 200 }; // 装備済みスロットの枠

	constexpr const char* MONO_FONT_NAME{ "Cascadia Mono" };
	constexpr const char* EMPTY_LABEL{ "--" };

	// 拡張子アイコンの画像ID（resources.json）。セレクト画面と同じ絵柄を128pxへ縮小したもの
	constexpr const char* EMPTY_ICON_IMAGE_ID{ "ext-emp" };
	constexpr std::array<std::pair<core::data::FileExtensionType, const char*>, 5> ICON_IMAGE_IDS{ {
		{ core::data::FileExtensionType::Executable, "ext-exe" },
		{ core::data::FileExtensionType::Document, "ext-doc" },
		{ core::data::FileExtensionType::Image, "ext-img" },
		{ core::data::FileExtensionType::Audio, "ext-aud" },
		{ core::data::FileExtensionType::Archive, "ext-arc" },
	} };

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
		case core::data::FileExtensionType::Archive: return "ARC";
		default: return "ETC";
		}
	}

	/**
	 * @brief 拡張子種別が何を強化するかを返す
	 *
	 * 装備の効果は開始時のステータス補正としてのみ現れるため、
	 * 種別名だけでは何の役に立っているのか分からない。効果を併記する
	 * @param type 拡張子種別
	 * @return 強化される項目の短い表記
	 */
	const char* toBonusLabel(core::data::FileExtensionType type)
	{
		switch (type)
		{
		case core::data::FileExtensionType::Executable: return "ATK+";
		case core::data::FileExtensionType::Document: return "SPD+";
		case core::data::FileExtensionType::Image: return "DEF+";
		case core::data::FileExtensionType::Audio: return "HP+";
		case core::data::FileExtensionType::Archive: return "ALL+";
		default: return "RNG+";
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
		for (const auto& [type, imageId] : ICON_IMAGE_IDS)
		{
			const int handle{ resourceManager.loadImageById(imageId) };
			if (handle == -1)
				core::log::error("装備スロットの拡張子アイコン '{}' の読み込みに失敗しました", imageId);
			m_iconHandles[static_cast<int>(type)] = handle;
		}

		m_emptyIconHandle = resourceManager.loadImageById(EMPTY_ICON_IMAGE_ID);
		if (m_emptyIconHandle == -1)
			core::log::error("装備スロットの空きアイコン '{}' の読み込みに失敗しました", EMPTY_ICON_IMAGE_ID);
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
			drawSlot(x, y, slotSize, m_equipmentData.getExtensionType(i), m_equipmentData.hasSelection(i));
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

		m_uiRenderer.setFont(MONO_FONT_NAME);

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

		// Unknown には専用アイコンが無い。その場合だけ種別名を文字で見せる
		const int iconHandle{ getIconHandle(type) };
		if (iconHandle != -1)
			m_uiRenderer.drawImage(iconHandle, iconX, iconY, iconSize, iconSize);
		else
			drawCenteredText(centerX, y + scaled(TYPE_LABEL_Y), toTypeLabel(type),
			    core::utility::Color::HUD_INK, scaled(TYPE_FONT_SIZE));

		drawCenteredText(centerX, y + scaled(BONUS_LABEL_Y), toBonusLabel(type),
		    core::utility::Color::HUD_INK_FAINT, scaled(BONUS_FONT_SIZE));

		m_uiRenderer.resetFont();
	}

	void EquipmentSlotView::drawCenteredText(int centerX, int y, const char* text, unsigned int color, int fontSize)
	{
		const int width{ m_uiRenderer.getTextWidth(text, fontSize) };
		m_uiRenderer.drawText(centerX - width / 2, y, text, color, fontSize);
	}
} // namespace game::ui::ingame
