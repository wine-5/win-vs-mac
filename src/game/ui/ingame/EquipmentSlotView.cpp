#include "EquipmentSlotView.h"
#include "core/constant/UI.h"
#include "core/interface/IResourceManager.h"
#include "core/utility/Color.h"
#include "core/utility/Log.h"
#include "game/data/FileEquipmentData.h"
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

	// 縁を周回する光の粒（装備中のスロットのみ）
	constexpr float ORBIT_PERIOD{ 3.2f };          // 一周にかける秒数
	constexpr int ORBIT_COMET_COUNT{ 2 };          // 同時に回る粒の列の数（外周上で等間隔に配置する）
	constexpr int ORBIT_TRAIL_COUNT{ 16 };         // 1列あたりの粒の数（後ろほど淡くなる）
	constexpr float ORBIT_TRAIL_SPACING{ 0.011f }; // 粒どうしの間隔（周回全体を1.0とした割合）
	constexpr int ORBIT_DOT_RADIUS{ 3 };           // 先頭の粒の半径（1080p基準）
	constexpr int ORBIT_ALPHA{ 210 };              // 加算合成の強さ（粒ごとの明暗は色側で付ける）
	constexpr float ORBIT_PHASE_PER_SLOT{ 0.33f }; // スロットごとに位相をずらして同期させない

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

	/**
	 * @brief 色の明るさを倍率で落とす
	 *
	 * 加算合成では色を暗くすることが透明度を下げることと同じ意味になる。
	 * 粒ごとにブレンドモードを設定し直さずに済ませるため、明暗は色側で付ける
	 * @param color 元の色（ARGB形式：0xAARRGGBB）
	 * @param scale 明るさの倍率（0.0〜1.0）
	 * @return 暗くした色
	 */
	unsigned int scaleBrightness(unsigned int color, float scale)
	{
		auto channel = [&](int shift)
		{ return static_cast<int>(((color >> shift) & 0xFFu) * scale); };
		return core::utility::Color::argb(255, channel(16), channel(8), channel(0));
	}

	/**
	 * @brief 正方形の外周上の点を求める
	 *
	 * 角丸ぶんのズレは半径4pxと小さく、粒が角を通る一瞬しか出ないため無視する
	 * @param x 左上のX座標
	 * @param y 左上のY座標
	 * @param size 一辺の長さ
	 * @param t 外周をひと回りする進行度（0.0〜1.0。0.0が左上で時計回り）
	 * @param outX 求めたX座標の格納先
	 * @param outY 求めたY座標の格納先
	 */
	void pointOnSquarePerimeter(int x, int y, int size, float t, int& outX, int& outY)
	{
		// 0.0〜1.0の範囲へ丸めてから、上→右→下→左の4辺に割り当てる
		const float wrapped{ t - std::floor(t) };
		const float edge{ wrapped * 4.0f };
		const int side{ static_cast<int>(edge) };
		const int along{ static_cast<int>((edge - side) * size) };

		switch (side)
		{
		case 0:
			outX = x + along;
			outY = y;
			break; // 上辺（左→右）
		case 1:
			outX = x + size;
			outY = y + along;
			break; // 右辺（上→下）
		case 2:
			outX = x + size - along;
			outY = y + size;
			break; // 下辺（右→左）
		default:
			outX = x;
			outY = y + size - along;
			break; // 左辺（下→上）
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
			const bool hasSelection{ m_equipmentData.hasSelection(i) };
			drawSlot(x, y, slotSize, m_equipmentData.getExtensionType(i), hasSelection);

			// 装備中のスロットだけ縁を光の粒が回り続ける（起動中であることの表現）
			if (hasSelection)
				drawOrbitingGlow(x, y, slotSize, i * ORBIT_PHASE_PER_SLOT);
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

	void EquipmentSlotView::drawOrbitingGlow(int x, int y, int size, float phaseOffset)
	{
		const float elapsed{ std::chrono::duration<float>(
			std::chrono::steady_clock::now() - m_startTime)
			    .count() };
		const float head{ elapsed / ORBIT_PERIOD + phaseOffset };

		const int dotRadius{ std::max(2, scaled(ORBIT_DOT_RADIUS)) };

		// 加算合成で重ねると、粒が枠線の上を通るときに芯が白く抜けて発光して見える。
		// 粒ごとの明暗はアルファではなく色で付けるため、ブレンドの設定は1回で済む
		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ADD, ORBIT_ALPHA);

		for (int comet{ 0 }; comet < ORBIT_COMET_COUNT; ++comet)
		{
			// 列を外周上で等間隔に散らす（2列なら向かい合う位置になる）
			const float cometHead{ head + static_cast<float>(comet) / ORBIT_COMET_COUNT };

			for (int i{ 0 }; i < ORBIT_TRAIL_COUNT; ++i)
			{
				// 後続ほど過去の位置に置き、暗く小さくして尾を引かせる
				const float fade{ 1.0f - static_cast<float>(i) / ORBIT_TRAIL_COUNT };

				int dotX{ 0 };
				int dotY{ 0 };
				pointOnSquarePerimeter(x, y, size, cometHead - i * ORBIT_TRAIL_SPACING, dotX, dotY);

				m_uiRenderer.drawCircle(dotX, dotY, std::max(1, static_cast<int>(dotRadius * fade)),
				    scaleBrightness(core::utility::Color::HUD_CHARGE_CYAN, fade * fade), true, 1);
			}
		}

		m_uiRenderer.resetBlendMode();
	}

	void EquipmentSlotView::drawCenteredText(int centerX, int y, const char* text, unsigned int color, int fontSize)
	{
		const int width{ m_uiRenderer.getTextWidth(text, fontSize) };
		m_uiRenderer.drawText(centerX - width / 2, y, text, color, fontSize);
	}
} // namespace game::ui::ingame
