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
#include "game/utility/PlayerStats.h"
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

	// セレクト画面で選んだ枠は道中では変えられない。インベントリの固定枠と同じ赤で示し、
	// 「触れない枠」の色を画面ごとにずらさない
	constexpr unsigned int LOCKED_BORDER_COLOR{ core::utility::Color::HUD_LOCKED_RED };

	// RAMブロックで増えた枠。強化を示す緑で、もともとの枠と見分ける
	constexpr unsigned int GAINED_BORDER_COLOR{ core::utility::Color::HUD_BUFF_GREEN };

	// 倍率が掛かっているときに光の粒を速める割合。
	// 上げすぎると粒が線に見えて本数が数えられなくなり、倍率の手掛かりが消える
	constexpr float BOOSTED_SPEED_SCALE{ 1.5f };

	// 意味のある縁は太くする。1pxのままだと色を付けても背景に紛れて読めない
	constexpr int MEANINGFUL_BORDER_THICKNESS{ 3 };
	constexpr int MEANINGFUL_BORDER_ALPHA{ 255 };

	// 上下2段に分けたときの段の間隔
	constexpr int ROW_GAP{ 12 };

	constexpr const char* EMPTY_LABEL{ "--" };

	// 表示するページ。3枠しか置けないため、持ち込みと道中で拾ったぶんを交互に見せる。
	// 枠を6つ並べると視界の右下がふさがり、戦闘中に見えない場所が増える
	constexpr int PAGE_CARRIED{ 0 };  // セレクト画面で選んだもの
	constexpr int PAGE_ACQUIRED{ 1 }; // 道中で拾ったもの
	constexpr int PAGE_COUNT{ 2 };
	constexpr float PAGE_INTERVAL{ 5.0f }; // 自動で切り替わる間隔（秒。左下HUDと揃える）

	// 枠が増えたときに「道中で取得」を出しっぱなしにする長さ（秒）。
	// 自動送りに任せると、増えた直後に持ち込みのページが出て見逃す
	constexpr float SLOT_GAINED_HOLD{ 4.0f };

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

		// 枠が増えた直後は自動送りを止めて「道中で取得」に固定する。
		// RAMブロックは何も落とさないので、ここを見せないと壊した手応えが残らない
		const bool isSlotGained{ updateSlotGained(static_cast<int>(acquired.size())) };
		const int page{ isSlotGained ? PAGE_ACQUIRED
			                         : (hasAcquired
			                                   ? static_cast<int>(elapsed / PAGE_INTERVAL) % PAGE_COUNT
			                                   : 0) };

		// ページが変わった直後だけ下から持ち上げる。送っていないときは動かさない。
		// 枠が増えたときも同じ動きで入れて、切り替わったことを目に留める
		const float sincePageChange{ isSlotGained
			                             ? std::chrono::duration<float>(
			                                   std::chrono::steady_clock::now() - m_slotGainedTime)
			                                   .count()
			                             : std::fmod(elapsed, PAGE_INTERVAL) };
		const float slideProgress{ (hasAcquired || isSlotGained)
			                           ? std::min(sincePageChange / SLIDE_DURATION, 1.0f)
			                           : 1.0f };
		const int slideOffset{ static_cast<int>(scaled(SLIDE_OFFSET) * (1.0f - slideProgress)) };

		// 表示する枠の数はページで変わる。持ち込みは3つ固定、
		// 道中で拾ったぶんは増えた枠の数だけ並ぶ
		const int slotCount{ page == PAGE_CARRIED
			                     ? data::FileEquipmentData::MAX_SLOTS
			                     : static_cast<int>(acquired.size()) };

		// もともとの3枠を下段に、増えたぶんを上段へ載せる。
		//     [][]
		//   [][][]
		// 横一列に伸ばすと右下のかたまりが広がって視界を削るうえ、
		// 段を分けることで「下がもともと、上が増えたぶん」と役割も読める
		constexpr int BASE_SLOT_COUNT{ data::FileEquipmentData::MAX_SLOTS };
		const int bottomCount{ std::min(slotCount, BASE_SLOT_COUNT) };
		const int topCount{ slotCount - bottomCount };

		const int baseSlotSize{ scaled(SLOT_SIZE) };
		const int gap{ scaled(SLOT_GAP) };
		const int totalWidth{ baseSlotSize * BASE_SLOT_COUNT + gap * (BASE_SLOT_COUNT - 1) };

		// 段に4つ以上並ぶときだけ縮めて幅に収める。並び全体の幅は変えない
		const auto rowSlotSize = [&](int count)
		{
			return count <= BASE_SLOT_COUNT ? baseSlotSize
			                                : (totalWidth - gap * (count - 1)) / count;
		};

		const int bottomSize{ rowSlotSize(bottomCount) };
		const int topSize{ topCount > 0 ? rowSlotSize(topCount) : 0 };

		// 右下アンカー。幅はモニタのアスペクト比で変わるため必ず実際の画面幅から逆算する。
		// 見出しは並びの下へ置く。上に置くと段数によって高さが変わり、
		// ページが替わるたびに文字だけが上下して読みづらい
		const int right{ m_screen.getWidth() - scaled(MARGIN) };
		const int labelHeight{ scaled(PAGE_LABEL_FONT_SIZE) + scaled(PAGE_LABEL_PADDING_Y) * 2 };
		const int labelTop{ m_screen.getHeight() - scaled(MARGIN) - labelHeight + slideOffset };

		const int bottomY{ labelTop - scaled(PAGE_LABEL_GAP) - bottomSize };
		const int topY{ bottomY - scaled(ROW_GAP) - topSize };

		// 段ごとに中央へ寄せる。上段が少ないときに左端へ寄ると、
		// 下段との関係が崩れて別の並びに見える
		const auto drawRow = [&](int firstIndex, int count, int size, int rowY, SlotAccent accent)
		{
			if (count <= 0)
				return;

			const int rowWidth{ size * count + gap * (count - 1) };
			const int rowLeft{ right - totalWidth + (totalWidth - rowWidth) / 2 };

			for (int i{ 0 }; i < count; ++i)
			{
				const int index{ firstIndex + i };
				const int x{ rowLeft + i * (size + gap) };

				const bool isCarriedPage{ page == PAGE_CARRIED };
				const auto type{ isCarriedPage ? m_equipmentData.getExtensionType(index)
					                           : acquired[index] };
				const bool hasSelection{ isCarriedPage
					                         ? m_equipmentData.hasSelection(index)
					                         : type != core::data::FileExtensionType::Count };

				drawSlot(x, rowY, size, type, hasSelection, accent);

				// 装備中のスロットだけ縁を光の粒が回り続ける（起動中であることの表現）。
				// 色は縁と揃える。別の色で回すと、1つのマスが2つの色を主張してしまう。
				// ただし倍率が掛かっているときだけは紫で回す。滅多に無い状態なので、
				// 縁の意味より「いま特別な状態だ」を優先して伝える
				if (hasSelection)
					drawOrbitingGlow(x, rowY, size, index * orbit_glow::PHASE_PER_SLOT,
					    isBonusBoosted() ? core::utility::Color::HUD_EXTENSION_BOOST_VIOLET
					                     : borderColor(hasSelection, accent));
			}
		};

		const SlotAccent bottomAccent{ page == PAGE_CARRIED ? SlotAccent::Locked
			                                                : SlotAccent::Normal };

		drawPageLabel(right - totalWidth, labelTop + scaled(PAGE_LABEL_PADDING_Y),
		    totalWidth, page);

		drawRow(0, bottomCount, bottomSize, bottomY, bottomAccent);
		drawRow(bottomCount, topCount, topSize, topY, SlotAccent::Gained);
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

	void EquipmentSlotView::drawSlot(int x, int y, int size, core::data::FileExtensionType type,
	    bool hasSelection, SlotAccent accent)
	{
		// マスを小さく描くときは中身も一緒に縮める。枠だけ詰めると中身がはみ出す
		const auto fit = [this, size](int scaledValue)
		{ return scaledValue * size / std::max(1, scaled(SLOT_SIZE)); };

		const int radius{ fit(scaled(SLOT_RADIUS)) };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, SLOT_FILL_ALPHA);
		m_uiRenderer.drawRoundedBox(x, y, size, size, radius, SLOT_FILL_COLOR, true, 1);

		// 意味のある縁は太く濃くする。1pxのままだと色を付けても背景に紛れて読めない
		const bool isMeaningful{ hasSelection || accent != SlotAccent::Normal };
		const int borderAlpha{ isMeaningful ? MEANINGFUL_BORDER_ALPHA : SLOT_BORDER_ALPHA };
		const int thickness{ isMeaningful ? scaled(MEANINGFUL_BORDER_THICKNESS) : 1 };

		m_uiRenderer.setBlendMode(core::constant::ui::BLEND_MODE_ALPHA, borderAlpha);
		m_uiRenderer.drawRoundedBox(x, y, size, size, radius,
		    borderColor(hasSelection, accent), false, thickness);
		m_uiRenderer.resetBlendMode();

		const int centerX{ x + size / 2 };
		const int iconSize{ fit(scaled(ICON_SIZE)) };
		const int iconX{ centerX - iconSize / 2 };
		const int iconY{ y + fit(scaled(ICON_Y)) };

		m_uiRenderer.setFont(core::constant::ui::MONO_FONT_NAME);

		if (!hasSelection)
		{
			// 空きスロットは点線枠のアイコンで示す。画像が無ければ文字で代替する
			if (m_emptyIconHandle != -1)
				m_uiRenderer.drawImage(m_emptyIconHandle, iconX, iconY, iconSize, iconSize);
			else
				drawCenteredText(centerX, y + fit(scaled(TYPE_LABEL_Y)), EMPTY_LABEL,
				    core::utility::Color::HUD_INK_FAINT, fit(scaled(TYPE_FONT_SIZE)));
			m_uiRenderer.resetFont();
			return;
		}

		// 全種別にアイコンがある。読み込みに失敗したときだけ種別名を文字で見せる
		const int iconHandle{ getIconHandle(type) };
		if (iconHandle != -1)
			m_uiRenderer.drawImage(iconHandle, iconX, iconY, iconSize, iconSize);
		else
			drawCenteredText(centerX, y + fit(scaled(TYPE_LABEL_Y)), toTypeLabel(type),
			    core::utility::Color::HUD_INK, fit(scaled(TYPE_FONT_SIZE)));

		// 何を強化するかを併記する。装備の効果は開始時のステータス補正としてのみ
		// 現れるため、種別名だけでは何の役に立っているのか分からない
		const std::string bonusLabel{ utility::ExtensionBonusLabel::toLabel(type) };
		drawCenteredText(centerX, y + fit(scaled(BONUS_LABEL_Y)), bonusLabel.c_str(),
		    core::utility::Color::HUD_INK_FAINT, fit(scaled(BONUS_FONT_SIZE)));

		m_uiRenderer.resetFont();
	}

	unsigned int EquipmentSlotView::borderColor(bool hasSelection, SlotAccent accent) const
	{
		// 道中では変えられない枠は赤、増えた枠は緑。中身の有無に関わらず役割を優先する
		if (accent == SlotAccent::Locked)
			return LOCKED_BORDER_COLOR;
		if (accent == SlotAccent::Gained)
			return GAINED_BORDER_COLOR;

		// 装備済みはアクセント色。空きは薄いままにして視線を集めない
		return hasSelection ? core::utility::Color::HUD_ACCENT : SLOT_BORDER_COLOR;
	}

	bool EquipmentSlotView::updateSlotGained(int maxEquipped)
	{
		// 初回は基準を取るだけ。ここで演出を始めると、開始直後に必ず一度流れてしまう
		if (m_previousMaxEquipped < 0)
		{
			m_previousMaxEquipped = maxEquipped;
			return false;
		}

		if (maxEquipped > m_previousMaxEquipped)
		{
			m_slotGainedTime = std::chrono::steady_clock::now();
			m_isSlotGained = true;
		}
		m_previousMaxEquipped = maxEquipped;

		if (!m_isSlotGained)
			return false;

		const float since{ std::chrono::duration<float>(
			std::chrono::steady_clock::now() - m_slotGainedTime)
			    .count() };
		if (since >= SLOT_GAINED_HOLD)
			m_isSlotGained = false;

		return m_isSlotGained;
	}

	bool EquipmentSlotView::isBonusBoosted() const
	{
		return utility::playerBonusMultiplier(m_componentManager, m_playerId) > 1.0f;
	}

	void EquipmentSlotView::drawOrbitingGlow(int x, int y, int size, float phaseOffset,
	    unsigned int color)
	{
		float elapsed{ std::chrono::duration<float>(
			std::chrono::steady_clock::now() - m_startTime)
			    .count() };

		// 倍率が掛かっているときは列を倍にして速さも上げる。
		// 列の本数は数えられるので「2列→4列＝2倍」と理屈が通る。
		// 速さは単独では気付けない（比べる相手が無い）が、本数と併せると勢いが乗る
		const bool boosted{ isBonusBoosted() };
		int cometCount{ orbit_glow::COMET_COUNT };
		if (boosted)
		{
			cometCount = orbit_glow::COMET_COUNT_BOOSTED;
			elapsed *= BOOSTED_SPEED_SCALE;
		}

		orbit_glow::draw(m_uiRenderer, x, y, size, size, elapsed, phaseOffset,
		    scaled(orbit_glow::DOT_RADIUS), color, cometCount);
	}

	void EquipmentSlotView::drawCenteredText(int centerX, int y, const char* text, unsigned int color, int fontSize)
	{
		const int width{ m_uiRenderer.getTextWidth(text, fontSize) };
		m_uiRenderer.drawText(centerX - width / 2, y, text, color, fontSize);
	}
} // namespace game::ui::ingame
